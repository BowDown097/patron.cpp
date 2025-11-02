#include "moduleservice.h"

namespace patron
{
std::vector<const command_info*> module_service::search_command(std::string_view name) const
{
    std::vector<const command_info*> out;

    for (const std::unique_ptr<module_base>& module : m_modules)
        for (const auto& cmd : module->commands)
            if (cmd.name() == name)
                out.push_back(&cmd);

    return out;
}

const module_base* module_service::search_module(std::string_view name) const
{
    for (const std::unique_ptr<module_base>& module : m_modules)
        if (module->name == name)
            return module.get();
    return nullptr;
}
}
