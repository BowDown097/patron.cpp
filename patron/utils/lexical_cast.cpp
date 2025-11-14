#include "lexical_cast.h"
#include "strings.h"

namespace patron
{
    namespace utility
    {
        bad_lexical_cast::bad_lexical_cast(std::string_view sourceTypeName, std::string_view targetTypeName)
            : message(targetTypeName.empty()
                  ? "Failed to convert from " + demangle(sourceTypeName)
                  : "Failed to convert from " + demangle(sourceTypeName) +
                    " to " + demangle(targetTypeName)) {}
    }
}
