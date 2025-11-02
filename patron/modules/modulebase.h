#pragma once
#include "patron/commands/commandinfo.h"

namespace patron
{
struct module_base
{
    std::vector<command_info> commands;
    std::string_view name;
};
}
