#pragma once
#include "lexical_cast.h"
#include <functional>

namespace patron
{
    namespace utility
    {
        template<typename T>
        concept StringViewOrChar = std::is_convertible_v<T, std::string_view> || std::same_as<T, char>;

        template<std::ranges::range R, class F>
            requires std::same_as<std::invoke_result_t<F&, std::ranges::range_reference_t<R>>, std::string>
        constexpr std::string join(R&& range, StringViewOrChar auto&& delim, F&& func)
        {
            if (std::ranges::empty(range))
                return std::string();

            std::string init = std::invoke(func, *std::ranges::begin(range));
            for (auto it = std::next(std::ranges::begin(range)); it != std::ranges::end(range); ++it)
            {
                init += delim;
                init += std::invoke(func, *it);
            }

            return init;
        }

        constexpr std::string join(std::ranges::range auto&& range, StringViewOrChar auto&& delim)
        {
            if (std::ranges::empty(range))
                return std::string();

            if constexpr (StringViewOrChar<std::ranges::range_value_t<decltype(range)>>)
            {
                std::string init(*std::ranges::begin(range));
                for (auto it = std::next(std::ranges::begin(range)); it != std::ranges::end(range); ++it)
                {
                    init += delim;
                    init += *it;
                }
                return init;
            }
            else
            {
                std::string init = lexical_cast<std::string>(*std::ranges::begin(range));
                for (auto it = std::next(std::ranges::begin(range)); it != std::ranges::end(range); ++it)
                {
                    init += delim;
                    init += lexical_cast<std::string>(*it);
                }
                return init;
            }
        }
    }
}
