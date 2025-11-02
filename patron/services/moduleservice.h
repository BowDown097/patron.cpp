#pragma once
#include "patron/modules/modulebase.h"
#include <memory>

namespace patron
{
class module_service
{
public:
    std::span<const std::unique_ptr<module_base>> modules() const { return m_modules; }
    std::vector<const command_info*> search_command(std::string_view name) const;
    const module_base* search_module(std::string_view name) const;

    template<std::derived_from<module_base> M>
    void register_module()
    {
        std::unique_ptr<M> module = std::make_unique<M>();
        module->name = std::meta::identifier_of(^^M);

        constexpr std::meta::access_context ctx = std::meta::access_context::current();
        template for (constexpr std::meta::info member : define_static_array(std::meta::members_of(^^M, ctx)))
        {
            if constexpr (std::meta::is_function(member))
            {
                constexpr std::optional<std::meta::info> cmd_opt =
                    utility::find_annotation(member, ^^command);
                if constexpr (cmd_opt.has_value())
                {
                    constexpr command_info::command_data data(
                        cmd_opt.value(),
                        utility::find_annotation(member, ^^summary),
                        utility::find_annotation(member, ^^remarks),
                        utility::find_annotation(member, ^^alias),
                        build_usage(member));

                    module->commands.emplace_back(data, module.get());
                }
            }
        }

        m_modules.push_back(std::move(module));
    }
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
};
}
