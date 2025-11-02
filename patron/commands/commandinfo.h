#pragma once
#include "annotations.h"

namespace patron
{
struct module_base;

class command_info
{
    friend class module_service;

    struct command_data
    {
        std::string_view m_name;
        bool m_ignore_extra_args;
        std::string_view m_summary;
        std::string_view m_remarks;
        std::span<const char*> m_aliases;
        std::string_view m_usage;

        consteval command_data(
            std::meta::info command_info,
            std::optional<std::meta::info> summary_info,
            std::optional<std::meta::info> remarks_info,
            std::optional<std::meta::info> alias_info,
            utility::static_string_view usage)
            : m_summary(utility::extract_text<patron::summary>(summary_info, &summary::text)),
              m_remarks(utility::extract_text<patron::remarks>(remarks_info, &remarks::text)),
              m_aliases(utility::extract_span<patron::alias>(alias_info, &alias::aliases)),
              m_usage(usage)
        {
            command cmd = std::meta::extract<command>(command_info);
            m_name = std::string_view(cmd.text);
            m_ignore_extra_args = cmd.ignore_extra_args;
        }
    };
public:
    command_info(const command_data& data, const module_base* module)
        : m_data(data), m_module(module) {}

    std::string_view name() const { return m_data.m_name; }
    std::string_view summary() const { return m_data.m_summary; }
    std::string_view remarks() const { return m_data.m_remarks; }
    std::span<const char*> aliases() const { return m_data.m_aliases; }
    std::string_view usage() const { return m_data.m_usage; }
    const module_base* module() const { return m_module; }
private:
    command_data m_data;
    const module_base* m_module;
};
}
