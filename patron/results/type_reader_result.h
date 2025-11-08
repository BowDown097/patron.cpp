#pragma once
#include "result.h"

namespace patron
{
    class type_reader_result : public result
    {
    public:
        static type_reader_result from_success(std::string_view message = "")
        { return type_reader_result(std::nullopt, message); }

        static type_reader_result from_error(command_error error, std::string_view message)
        { return type_reader_result(error, message); }

        type_reader_result() = default;
    private:
        type_reader_result(const std::optional<command_error>& error, std::string_view message)
            : result(error, message) {}
    };
}
