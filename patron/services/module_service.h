#pragma once
#include "patron/commands/command_execution.h"
#include "patron/modules/module_base.h"

namespace patron
{
    template<template<typename> typename CoroutineTaskType = detail::no_task>
        requires (utility::specialization_of<CoroutineTaskType<void>, detail::no_task> ||
                  utility::is_awaitable<CoroutineTaskType<command_result>>)
    class module_service : public module_service_base
    {
        using command_result_t = std::conditional_t<
            utility::is_awaitable<CoroutineTaskType<command_result>>,
            CoroutineTaskType<command_result>,
            command_result>;
    public:
        explicit module_service(module_service_config config = {})
            : module_service_base(std::move(config)) {}

        std::vector<const module_base*> modules() const
        {
            std::vector<const module_base*> out;
            for (const std::unique_ptr<module_base>& module : m_modules)
                out.push_back(module.get());
            return out;
        }

        std::vector<const command_info*> search_command(std::string_view name) const
        {
            std::vector<const command_info*> out;

            for (const std::unique_ptr<module_base>& module : m_modules)
                for (const command_info& cmd : module->commands())
                    if (cmd.matches(name, config().case_sensitive_lookup))
                        out.push_back(&cmd);

            return out;
        }

        const module_base* search_module(std::string_view name) const
        {
            for (const std::unique_ptr<module_base>& module : m_modules)
                if (module->matches(name, config().case_sensitive_lookup))
                    return module.get();
            return nullptr;
        }

        template<std::derived_from<module_base> M>
        void register_module()
        {
            m_modules.push_back(create_module<M>());
        }

        template<std::meta::info NS> requires (std::meta::is_namespace(NS))
        void register_namespace()
        {
            constexpr std::meta::access_context ctx = std::meta::access_context::current();
            template for (constexpr std::meta::info member : define_static_array(std::meta::members_of(NS, ctx)))
            {
                if constexpr (std::meta::is_complete_type(member))
                {
                    using member_type = [:member:];
                    if constexpr (std::derived_from<member_type, module_base>)
                        register_module<member_type>();
                    else if constexpr (utility::specialization_of<member_type, type_reader>)
                        register_type_reader<member_type>();
                }
            }
        }
    protected:
        // TODO: provide implementation
        CoroutineTaskType<command_result> run_command(std::string_view name, std::span<const std::string> args)
            requires utility::is_awaitable<CoroutineTaskType<command_result>>;
        command_result run_command(std::string_view name, std::span<const std::string> args)
            requires (!utility::is_awaitable<CoroutineTaskType<command_result>>);
    private:
        std::vector<std::unique_ptr<module_base>> m_modules;

        static consteval std::string build_usage(std::meta::info command)
        {
            std::string result;
            for (std::meta::info p : std::meta::parameters_of(command))
            {
                if (std::meta::has_default_argument(p))
                {
                    result += '<';
                    result += std::meta::identifier_of(p);
                    result += '>';
                }
                else
                {
                    result += '[';
                    result += std::meta::identifier_of(p);
                    result += ']';
                }
                result += ' ';
            }

            if (!result.empty())
                result.pop_back();

            return result;
        }

        template<std::derived_from<module_base> M>
        std::unique_ptr<M> create_module()
        {
            constexpr module_base::module_data module_data(
                std::meta::identifier_of(^^M),
                utility::find_annotation(^^M, ^^summary),
                utility::find_annotation(^^M, ^^remarks),
                utility::find_annotation(^^M, ^^alias));

            std::unique_ptr<M> module = std::make_unique<M>();
            module->m_data = module_data;

            constexpr std::meta::access_context ctx = std::meta::access_context::current();
            template for (constexpr std::meta::info member : define_static_array(std::meta::members_of(^^M, ctx)))
            {
                if constexpr (std::meta::is_function(member))
                {
                    constexpr std::optional<std::meta::info> cmd_opt =
                        utility::find_annotation(member, ^^command);
                    if constexpr (cmd_opt.has_value())
                    {
                        constexpr command_info::command_data cmd_data(
                            cmd_opt.value(),
                            utility::find_annotation(member, ^^summary),
                            utility::find_annotation(member, ^^remarks),
                            utility::find_annotation(member, ^^alias),
                            build_usage(member));
                        command_function cmd_fn = command_execution::create_command_function
                            <member, utility::static_span<const std::meta::info>(std::meta::parameters_of(member)), M>
                            (cmd_data.m_name, cmd_data.m_ignore_extra_args, cmd_data.m_remainder);

                        module->m_commands.emplace_back(cmd_data, module.get(), std::move(cmd_fn));
                    }
                }
            }

            return module;
        }
    };
}
