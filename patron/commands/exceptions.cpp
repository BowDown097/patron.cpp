#include "exceptions.h"
#include <format>

namespace patron
{
    bad_argument_count::bad_argument_count(std::string_view command, std::size_t arg_count, std::size_t target_arg_count)
        : m_arg_count(arg_count), m_command(command), m_target_arg_count(target_arg_count),
          m_formatted_message(std::format(
              "{}: Ran with {} arguments, expects at least {}",
              command, arg_count, target_arg_count
          )) {}

    bad_command_argument::bad_command_argument(command_error error, std::string_view arg, std::size_t index,
                                               std::string_view command, std::string_view message)
        : m_arg(arg), m_command(command), m_error(error), m_index(index), m_message(message),
          m_formatted_message(std::format(
              "{}: Failed to convert argument {} ({}): {}",
              command, index, arg, message
          )) {}
}
