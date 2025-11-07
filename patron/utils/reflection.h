#pragma once
#include <meta>

namespace patron
{
    namespace utility
    {
        template<typename T>
        struct static_span
        {
            T* data;
            std::size_t size;

            consteval static_span() noexcept : data(nullptr), size(0) {}

            consteval static_span(std::span<T> span)
            {
                if constexpr (std::same_as<std::decay_t<T>, const char*>)
                {
                    std::vector<const char*> strings;
                    for (std::string_view sv : span)
                        strings.push_back(define_static_string(sv));

                    std::span<const T> sa = define_static_array(strings);
                    data = const_cast<T*>(sa.data());
                    size = sa.size();
                }
                else
                {
                    std::span<const T> sa = define_static_array(span);
                    data = const_cast<T*>(sa.data());
                    size = sa.size();
                }
            }

            consteval static_span(std::initializer_list<T> il)
                : static_span(std::span(const_cast<T*>(il.begin()), const_cast<T*>(il.end()))) {}

            constexpr T* begin() const noexcept { return data; }
            constexpr T* end() const noexcept { return data + size; }

            template<std::size_t N>
            constexpr operator std::span<T, N>() const
            {
                return std::span(begin(), end());
            }
        };

        struct static_string_view
        {
            const char* data;
            std::size_t size;

            consteval static_string_view() noexcept : data(nullptr), size(0) {}

            template<typename T> requires std::constructible_from<std::string_view, T>
            consteval static_string_view(T s)
                : data(std::define_static_string(s)), size(std::string_view(s).size()) {}

            constexpr const char* begin() const noexcept { return data; }
            constexpr const char* end() const noexcept { return data + size; }

            constexpr operator std::string_view() const
            {
                return std::string_view(begin(), end());
            }
        };

        consteval std::optional<std::meta::info> find_annotation(std::meta::info r, std::meta::info a)
        {
            std::optional<std::meta::info> res;
            for (std::meta::info a : std::meta::annotations_of(r, a))
            {
                if (res && *res != a)
                    throw "inconsistent annotations";
                res = a;
            }

            return res;
        }

        template<typename T, typename Field>
        consteval auto extract_span(std::optional<std::meta::info> opt, Field field)
            -> decltype(std::span(std::declval<T>().*field))
        {
            if (!opt) return {};
            return std::span(std::meta::extract<T>(*opt).*field);
        }

        template<typename T, typename Field>
        consteval std::string_view extract_text(std::optional<std::meta::info> opt, Field field)
        {
            if (!opt) return {};
            return std::string_view(std::meta::extract<T>(*opt).*field);
        }
    }
}
