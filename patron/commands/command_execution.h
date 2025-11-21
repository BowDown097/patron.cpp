#pragma once
#include "command_function.h"
#include "patron/services/module_service_base.h"
#include "patron/utils/join.h"
#include "patron/utils/reflection.h"

namespace patron
{
    class command_execution
    {
    public:
        template<std::meta::info FnInfo, utility::static_span<const std::meta::info> Params, typename Module>
        static command_function create_command_function(std::string_view cmd, bool ignore_extra_args, bool remainder)
        {
            using Result = typename[:std::meta::return_type_of(FnInfo):];
            using Function = std::function<Result(Module*, std::span<const std::string>, module_service_base*)>;

            constexpr std::size_t argc = target_arg_count(Params);
            return command_function(Function([=](Module* module, std::span<const std::string> args, module_service_base* service) {
                return [=]<std::size_t... Is>(std::index_sequence<Is...>) {
                    return invoke_fn<Result>(std::mem_fn(&[:FnInfo:]), module,
                        convert_arg_at<Params, Is>(cmd, ignore_extra_args, remainder, argc, args, service)...);
                }(std::make_index_sequence<Params.size>());
            }), argc);
        }
    private:
        template<typename T>
        static T convert_arg(const std::string& arg, std::size_t index, std::string_view cmd, module_service_base* service)
        {
            try
            {
                if constexpr (utility::specialization_of<T, std::optional>)
                {
                    if (arg.empty())
                        return std::nullopt;
                    return convert_arg<typename T::value_type>(arg, index, cmd, service);
                }
                else
                {
                    if (std::unique_ptr<type_reader_base<T>> reader = service->create_type_reader<T>())
                    {
                        if (type_reader_result result = reader->read(arg); result.success())
                            return reader->top_result();
                        else
                            throw bad_command_argument(result.error().value(), arg, index + 1, cmd, result.message());
                    }

                    if constexpr (requires { utility::lexical_cast<T>(std::declval<std::string>()); })
                        return utility::lexical_cast<T>(arg);
                    else
                        throw utility::bad_lexical_cast("std::string", typeid(T).name());
                }
            }
            catch (const utility::bad_lexical_cast& e)
            {
                throw bad_command_argument(command_error::parse_failed, arg, index + 1, cmd, e.what());
            }
        }

        template<utility::static_span<const std::meta::info> Params, std::size_t I>
        static auto convert_arg_at(std::string_view cmd, bool ignore_extra_args, bool remainder, std::size_t argc,
                                   std::span<const std::string> args, module_service_base* service)
        {
            using ArgType = [:std::meta::type_of(Params[I]):];
            if (I >= args.size())
            {
                if (ignore_extra_args)
                    return ArgType{};
                else
                    throw bad_argument_count(cmd, args.size(), argc);
            }

            if (remainder && I == Params.size - 1)
                return convert_arg<ArgType>(utility::join(args.subspan(I), ' '), I, cmd, service);
            else
                return convert_arg<ArgType>(args[I], I, cmd, service);
        }

        template<typename Result>
        static Result invoke_fn(auto&& fn, auto&&... args)
        {
            return std::invoke(std::forward<decltype(fn)>(fn), std::forward<decltype(args)>(args)...);
        }

        template<typename Result> requires utility::is_awaitable<Result>
        static Result invoke_fn(auto&& fn, auto&&... args)
        {
            if constexpr (std::is_void_v<utility::await_result_t<Result>>)
                co_await std::invoke(std::forward<decltype(fn)>(fn), std::forward<decltype(args)>(args)...);
            else
                co_return co_await std::invoke(std::forward<decltype(fn)>(fn), std::forward<decltype(args)>(args)...);
        }

        static consteval std::size_t target_arg_count(utility::static_span<const std::meta::info> params)
        {
            return std::ranges::count_if(params, [](std::meta::info p) {
                std::meta::info t = std::meta::type_of(p);
                return (!std::meta::has_template_arguments(t) || std::meta::template_of(t) != ^^std::optional) &&
                       !std::meta::has_default_argument(p);
            });
        }
    };
}
