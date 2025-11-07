#pragma once
#include "patron/commands/command_info.h"

namespace patron
{
    class module_base
    {
        template<typename C, typename E, template<typename> typename T>
            requires (utility::specialization_of<T<void>, detail::no_task> ||
                      utility::is_awaitable<T<command_result>>)
        friend class module_service;

        struct module_data
        {
            std::string_view m_name;
            std::string_view m_summary;
            std::string_view m_remarks;
            std::span<const char*> m_aliases;

            module_data() = default;

            consteval module_data(
                std::string_view name,
                std::optional<std::meta::info> summary_info,
                std::optional<std::meta::info> remarks_info,
                std::optional<std::meta::info> alias_info)
                : m_name(name),
                  m_summary(utility::extract_text<patron::summary>(summary_info, &summary::text)),
                  m_remarks(utility::extract_text<patron::remarks>(remarks_info, &remarks::text)),
                  m_aliases(utility::extract_span<patron::alias>(alias_info, &alias::aliases)) {}
        };
    public:
        virtual ~module_base() = default;

        bool matches(std::string_view str, bool case_sensitive) const;

        std::string_view name() const { return m_data.m_name; }
        std::string_view summary() const { return m_data.m_summary; }
        std::string_view remarks() const { return m_data.m_remarks; }
        std::span<const char*> aliases() const { return m_data.m_aliases; }
        std::span<const command_info> commands() const { return m_commands; }
    private:
        std::vector<command_info> m_commands;
        module_data m_data;
    };
}
