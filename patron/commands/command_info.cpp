#include "command_info.h"
#include "patron/utils/strings.h"

namespace patron
{
    bool command_info::matches(std::string_view str, bool case_sensitive) const
    {
        if (utility::sequals(str, name(), case_sensitive))
            return true;
        for (std::string_view alias : aliases())
            if (utility::sequals(str, alias, case_sensitive))
                return true;
        return false;
    }
}
