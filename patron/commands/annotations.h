#pragma once
#include "patron/utils/reflection.h"

namespace patron
{
struct alias
{
    utility::static_span<const char*> aliases;
};

struct command
{
    utility::static_string_view text;
    bool ignore_extra_args = true;
};

struct remarks
{
    utility::static_string_view text;
};

struct summary
{
    utility::static_string_view text;
};
}
