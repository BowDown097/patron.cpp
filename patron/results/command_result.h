#pragma once
#include "result.h"

namespace patron
{
    class command_result : public result
    {
    public:
        static command_result from_success(std::string_view message = "")
        { return command_result(std::nullopt, message); }

        static command_result from_error(std::string_view message = "")
        { return command_result(command_error::unsuccessful, message); }

        static command_result from_error(const std::exception& e)
        { return command_result(command_error::exception, e.what()); }

        static command_result from_error(command_error error, std::string_view message)
        { return command_result(error, message); }

        command_result() = default;
    private:
        command_result(const std::optional<command_error>& error, std::string_view message)
            : result(error, message) {}
    };
}
