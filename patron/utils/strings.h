#pragma once
#include <string>

namespace patron
{
    namespace utility
    {
        std::string demangle(std::string_view name);
        bool iequals(std::string_view s1, std::string_view s2);
        bool sequals(std::string_view s1, std::string_view s2, bool case_sensitive);
    }
}
