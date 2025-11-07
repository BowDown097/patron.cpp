#pragma once
#include "exceptions.h"
#include "patron/results/command_result.h"
#include "patron/utils/concepts.h"
#include <functional>

namespace patron
{
    struct variant_function_wrapper_base
    {
        virtual ~variant_function_wrapper_base() = default;
    };

    template<typename ReturnType, typename... Args>
    struct variant_function_wrapper : variant_function_wrapper_base
    {
        std::function<ReturnType(Args...)> func;
        explicit variant_function_wrapper(std::function<ReturnType(Args...)> f) : func(std::move(f)) {}
    };

    class command_function
    {
    public:
        template<typename ReturnType, typename... Args>
        command_function(std::function<ReturnType(Args...)> f, std::size_t target_arg_count)
            : m_target_arg_count(target_arg_count),
              m_wrapper(std::make_unique<variant_function_wrapper<ReturnType, Args...>>(std::move(f))) {}

        template<template<typename> typename TaskType = detail::no_task>
            requires utility::is_awaitable<TaskType<command_result>>
        TaskType<command_result> invoke_with_result(std::string_view name, std::size_t arg_count, bool exceptions, auto&&... args)
        {
            if (arg_count < m_target_arg_count)
            {
                bad_argument_count arg_ex(name, arg_count, m_target_arg_count);
                if (exceptions)
                    throw arg_ex;
                else
                    co_return command_result::from_error(arg_ex.error(), arg_ex.what());
            }

            if (exceptions)
            {
                co_return co_await invoke<TaskType<command_result>>(std::forward<decltype(args)>(args)...);
            }
            else
            {
                try
                {
                    co_return co_await invoke<TaskType<command_result>>(std::forward<decltype(args)>(args)...);
                }
                catch (const bad_command_argument& e)
                {
                    co_return command_result::from_error(e.error(), e.what());
                }
                catch (const std::exception& e)
                {
                    co_return command_result::from_error(e);
                }
            }
        }

        template<template<typename> typename TaskType = detail::no_task>
            requires (!utility::is_awaitable<TaskType<command_result>>)
        command_result invoke_with_result(std::string_view name, std::size_t arg_count, bool exceptions, auto&&... args)
        {
            if (arg_count < m_target_arg_count)
            {
                bad_argument_count arg_ex(name, arg_count, m_target_arg_count);
                if (exceptions)
                    throw arg_ex;
                else
                    return command_result::from_error(arg_ex.error(), arg_ex.what());
            }

            if (exceptions)
            {
                return invoke<command_result>(std::forward<decltype(args)>(args)...);
            }
            else
            {
                try
                {
                    return invoke<command_result>(std::forward<decltype(args)>(args)...);
                }
                catch (const bad_command_argument& e)
                {
                    return command_result::from_error(e.error(), e.what());
                }
                catch (const std::exception& e)
                {
                    return command_result::from_error(e);
                }
            }
        }
    private:
        std::size_t m_target_arg_count;
        std::unique_ptr<variant_function_wrapper_base> m_wrapper;

        template<typename T>
        T invoke(auto&&... args)
        {
            using WrapperType = variant_function_wrapper<T, std::remove_cvref_t<decltype(args)>...>;
            if (WrapperType* fw = dynamic_cast<WrapperType*>(m_wrapper.get()))
                return fw->func(std::forward<decltype(args)>(args)...);
            return T{};
        }

        template<utility::is_awaitable T>
        T invoke(auto&&... args)
        {
            using ValueType = utility::await_result_t<T>;
            using WrapperType = variant_function_wrapper<T, std::remove_cvref_t<decltype(args)>...>;

            if (WrapperType* fw = dynamic_cast<WrapperType*>(m_wrapper.get()))
            {
                if constexpr (std::is_void_v<ValueType>)
                    co_await fw->func(std::forward<decltype(args)>(args)...);
                else
                    co_return co_await fw->func(std::forward<decltype(args)>(args)...);
            }

            co_return ValueType{};
        }
    };
}
