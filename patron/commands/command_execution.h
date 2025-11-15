#pragma once
#include "command_function.h"
#include "patron/utils/join.h"
#include "patron/utils/reflection.h"
#include "type_reader.h"
#include <algorithm>

namespace patron
{
    class command_execution
    {
    public:
        template<std::meta::info FnInfo, utility::static_span<const std::meta::info> Params, typename Module, typename Service>
        static command_function create_command_function(std::string_view cmd, bool ignore_extra_args, bool remainder)
        {
            using Event = typename Service::event_type;
            using Result = typename[:std::meta::return_type_of(FnInfo):];
            using Function = std::function<Result(Module*, std::span<const std::string>, const Event*, Service*)>;

            constexpr std::size_t argc = target_arg_count(Params);
            return command_function(Function([=](Module* module, std::span<const std::string> args, const Event* event, Service* service) {
                return [=]<std::size_t... Is>(std::index_sequence<Is...>) {
                    return invoke_fn<Result>(std::mem_fn(&[:FnInfo:]), module,
                        convert_arg_at<Params, Is>(cmd, ignore_extra_args, remainder, argc, args, event, service)...);
                }(std::make_index_sequence<Params.size>());
            }), argc);
        }
    private:
        template<typename Reader, typename Event, typename Service>
        static type_reader_result call_reader(Reader* reader, const std::string& arg, const Event* event, Service* service)
        {
            using ReaderContext = typename Reader::context_type;
            using ReaderEvent = typename Reader::event_type;
            if constexpr (std::is_void_v<ReaderContext> && std::is_void_v<ReaderEvent>)
                return reader->read(arg);
            else if constexpr (std::is_void_v<ReaderContext>)
                return reader->read(event, arg);
            else if constexpr (std::is_void_v<ReaderEvent>)
                return reader->read(service->context(), arg);
            else
                return reader->read(service->context(), event, arg);
        }

        template<typename T, typename Event, typename Service>
        static T convert_arg(const std::string& arg, std::size_t index, std::string_view cmd, const Event* event, Service* service)
        {
            try
            {
                if constexpr (utility::specialization_of<T, std::optional>)
                {
                    if (arg.empty())
                        return std::nullopt;
                    return convert_arg<typename T::value_type>(arg, index, cmd, event, service);
                }
                else
                {
                    if (std::optional<T> reader_result = try_get_type_reader_result<T>(service, event, arg, index, cmd))
                        return reader_result.value();

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

        template<utility::static_span<const std::meta::info> Params, std::size_t I, typename Event, typename Service>
        static auto convert_arg_at(std::string_view cmd, bool ignore_extra_args, bool remainder, std::size_t argc,
                                   std::span<const std::string> args, const Event* event, Service* service)
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
                return convert_arg<ArgType>(utility::join(args.subspan(I), ' '), I, cmd, event, service);
            else
                return convert_arg<ArgType>(args[I], I, cmd, event, service);
        }

        template<typename T, typename Service>
        static auto find_reader(Service* service)
        {
            using Context = typename Service::context_type;
            using Event = typename Service::event_type;
            if (auto reader = service->template create_type_reader<T, Context, Event>())
                return reader;
            if (auto reader = service->template create_type_reader<T, Context, void>())
                return reader;
            if (auto reader = service->template create_type_reader<T, void, Event>())
                return reader;
            return service->template create_type_reader<T, void, void>();
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

        template<typename T, typename Service, typename Event>
        static std::optional<T> try_get_type_reader_result(
            Service* service, const Event* event, const std::string& arg, std::size_t index, std::string_view cmd)
        {
            if (auto reader = find_reader<T>(service))
            {
                if (type_reader_result result = call_reader(reader.get(), arg, event, service); result.success())
                    return reader->top_result();
                else
                    throw bad_command_argument(result.error().value(), arg, index + 1, cmd, result.message());
            }

            return std::nullopt;
        }
    };
}
