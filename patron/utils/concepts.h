#pragma once
#include <coroutine>
#include <utility>

namespace patron
{
namespace detail
{
template<typename = void>
struct no_task {};
}

namespace utility
{
// general
template<typename T, template<typename...> typename Primary>
struct is_specialization_of : std::false_type {};
template<template<typename...> typename Primary, typename... Args>
struct is_specialization_of<Primary<Args...>, Primary> : std::true_type {};
template<typename T, template<typename...> typename Primary>
concept specialization_of = is_specialization_of<T, Primary>::value;

template<typename T, typename... Ts>
concept any_of = (std::same_as<T, Ts> || ...);

// coroutine
namespace detail
{
template<typename Awaitable>
auto get_awaiter(Awaitable&& awaitable, void*) -> decltype(auto)
{
    if constexpr (requires { std::forward<Awaitable>(awaitable).operator co_await(); })
        return std::forward<Awaitable>(awaitable).operator co_await();
    else if constexpr (requires { operator co_await(std::forward<Awaitable>(awaitable)); })
        return operator co_await(std::forward<Awaitable>(awaitable));
    else
        return std::forward<Awaitable>(awaitable);
}

template<typename Awaitable, typename Promise>
auto get_awaiter(Awaitable&& awaitable, Promise* promise) -> decltype(auto)
requires(requires { promise->await_transform(std::forward<Awaitable>(awaitable)); })
{
    return get_awaiter(promise->await_transform(
        std::forward<Awaitable>(awaitable),
        const_cast<void*>(static_cast<void const volatile*>(promise))));
}

template<typename Promise, typename Awaiter>
auto do_await_suspend(Awaiter& awaiter) -> decltype(auto)
{
    if constexpr (!std::same_as<Promise, void>)
        return awaiter.await_suspend(std::coroutine_handle<Promise>{});
}

template<typename T>
concept is_valid_await_suspend = any_of<T, void, bool> || specialization_of<T, std::coroutine_handle>;

template<typename T, typename Promise = void>
concept is_awaiter = requires(T& awaiter) {
    { awaiter.await_ready() } -> std::same_as<bool>;
    { do_await_suspend<Promise>(awaiter) } -> is_valid_await_suspend;
    awaiter.await_resume();
};
}

template<typename T, typename Promise = void>
concept is_awaitable = requires(T (&f)() noexcept, Promise* promise) {
    { detail::get_awaiter(f(), promise) } -> detail::is_awaiter<Promise>;
};
}
}
