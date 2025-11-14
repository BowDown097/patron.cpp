#pragma once
#include <charconv>
#include <format>
#include <sstream>

namespace patron
{
    namespace utility
    {
        namespace detail
        {
            template<typename T>
            concept StringViewLike = std::convertible_to<T, std::string_view>;
        }

        class bad_lexical_cast : public std::bad_cast
        {
        public:
            explicit bad_lexical_cast(std::string_view sourceTypeName, std::string_view targetTypeName = {});
            explicit bad_lexical_cast(std::string_view message) : message(message) {}
            const char* what() const noexcept override { return message.c_str(); }
        private:
            std::string message;
        };

        namespace casters
        {
            template<typename Target, typename Source>
            struct lexical_caster
            {
                static Target cast(const Source& s)
                {
                    std::stringstream ss;
                    if ((ss << s).fail())
                        throw bad_lexical_cast(typeid(Source).name());

                    Target t;
                    if ((ss >> t).fail() || !(ss >> std::ws).eof())
                        throw bad_lexical_cast(typeid(Source).name(), typeid(Target).name());

                    return t;
                }
            };

            template<typename T>
            struct lexical_caster<T, T>
            {
                static constexpr const T& cast(const T& s)
                {
                    return s;
                }
            };

            template<>
            struct lexical_caster<std::string, std::string>
            {
                static constexpr const std::string& cast(const std::string& s)
                {
                    return s;
                }
            };

            template<detail::StringViewLike StringViewLike>
            struct lexical_caster<std::string, StringViewLike>
            {
                static constexpr std::string cast(std::string_view s)
                {
                    return std::string(s);
                }
            };

            template<>
            struct lexical_caster<std::string, bool>
            {
                static constexpr std::string cast(bool b)
                {
                    return b ? "true" : "false";
                }
            };

            template<typename Source>
            struct lexical_caster<std::string, Source>
            {
                static std::string cast(const Source& s)
                {
                    if constexpr (std::formattable<Source, char>)
                    {
                        return std::format("{}", s);
                    }
                    else
                    {
                        std::ostringstream oss;
                        if ((oss << s).fail())
                            throw bad_lexical_cast(typeid(Source).name());
                        return oss.str();
                    }
                }
            };

            template<typename Number, detail::StringViewLike StringViewLike> requires std::is_arithmetic_v<Number>
            struct lexical_caster<Number, StringViewLike>
            {
                static constexpr Number cast(std::string_view s)
                {
                    Number n;
                    if (auto [_, ec] = std::from_chars(s.data(), s.data() + s.size(), n); ec != std::errc())
                        throw bad_lexical_cast("string", typeid(Number).name());
                    return n;
                }
            };

            template<typename Number> requires std::is_arithmetic_v<Number>
            struct lexical_caster<std::string, Number>
            {
                static constexpr std::string cast(Number n)
                {
                    // the "magic numbers" here are to leave room for other characters such as "+-e,."
                    // floating point types need a larger size to account for the decimal part
                    constexpr size_t bufsize = std::integral<Number>
                        ? std::numeric_limits<Number>::digits10 + 2
                        : std::numeric_limits<Number>::digits10 + std::numeric_limits<Number>::max_digits10 + 10;

                    char buf[bufsize];
                    const auto res = std::to_chars(buf, buf + bufsize, n);
                    if (res.ec != std::errc())
                        throw bad_lexical_cast(typeid(Number).name(), "string");

                    return std::string(buf, res.ptr);
                }
            };
        }

        template<typename Target, typename Source>
        inline constexpr Target lexical_cast(const Source& s, bool exceptions = true)
        {
            if (exceptions)
            {
                return casters::lexical_caster<Target, Source>::cast(s);
            }
            else
            {
                try
                {
                    return casters::lexical_caster<Target, Source>::cast(s);
                }
                catch (const bad_lexical_cast& e)
                {
                    return Target{};
                }
            }
        }
    }
}
