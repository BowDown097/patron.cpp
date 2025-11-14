#include "strings.h"
#include <algorithm>
#include <cctype>

#if defined(__has_include) && __has_include(<cxxabi.h>) && !defined(__GABIXX_CXXABI_H__)
# include <cxxabi.h>
# include <memory>
# define NEED_DEMANGLE
#endif

namespace patron
{
    namespace utility
    {
        std::string demangle(std::string_view name)
        {
        #ifdef NEED_DEMANGLE
            int status = -1;
            std::unique_ptr<char, decltype(&free)> demangled(
                abi::__cxa_demangle(name.data(), nullptr, nullptr, &status), free);
            return status == 0 ? std::string(demangled.get()) : std::string(name);
        #else
            return std::string(name);
        #endif
        }

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
