#include "strings.h"
#include <algorithm>

namespace patron
{
    namespace utility
    {
        bool iequals(std::string_view s1, std::string_view s2)
        {
            return std::ranges::equal(s1, s2, [](unsigned char a, unsigned char b) {
                return std::tolower(a) == std::tolower(b);
            });
        }

        bool sequals(std::string_view s1, std::string_view s2, bool case_sensitive)
        {
            return case_sensitive ? s1 == s2 : iequals(s1, s2);
        }
    }
}
