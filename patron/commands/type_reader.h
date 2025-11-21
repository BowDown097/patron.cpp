#pragma once
#include "patron/results/type_reader_result.h"
#include <algorithm>
#include <any>
#include <meta>
#include <span>
#include <vector>

namespace patron
{
    template<typename T>
    class type_reader_value
    {
    public:
        using value_type = T;

        template<typename U = T> requires std::is_constructible_v<T, U>
        constexpr type_reader_value(U&& value, float weight) noexcept(std::is_nothrow_constructible_v<T, U>)
            : m_value(std::forward<U>(value)), m_weight(weight) {}

        template<typename Self>
        constexpr auto operator->(this Self&& self) noexcept { return std::addressof(self.m_value); }

        template<typename Self>
        constexpr auto&& operator*(this Self&& self) noexcept { return std::forward<Self>(self).m_value; }

        template<typename Self>
        constexpr auto&& value(this Self&& self) noexcept { return std::forward<Self>(self).m_value; }

        constexpr float weight() const { return m_weight; }
    private:
        T m_value;
        float m_weight;
    };

    template<typename T>
    class type_reader_base
    {
    public:
        using value_type = T;

        virtual ~type_reader_base() = default;
        virtual type_reader_result read(const std::string& input) = 0;

        const T& top_result() const
        {
            if (m_results.empty())
                throw std::logic_error("Tried to get top result from type reader with no results");
            return std::ranges::max_element(m_results, {}, &type_reader_value<T>::weight)->value();
        }

        bool has_result() const { return !m_results.empty(); }
        explicit operator bool() const { return has_result(); }

        std::span<const type_reader_value<T>> results() const { return m_results; }
    protected:
        template<typename U = T> requires std::is_constructible_v<T, U>
        void add_result(U&& value, float weight = 1.0f)
        {
            m_results.emplace_back(std::forward<U>(value), weight);
        }
    private:
        std::vector<type_reader_value<T>> m_results;
    };

    template<typename Derived, typename T>
    struct type_reader : type_reader_base<T>
    {
        static type_reader<Derived, T>* create(std::span<const std::any> extra_data)
        {
            Derived* result = new Derived;

            constexpr std::meta::access_context ctx = std::meta::access_context::unchecked();
            template for (constexpr std::meta::info member : define_static_array(std::meta::nonstatic_data_members_of(^^Derived, ctx)))
            {
                using Member = [:std::meta::type_of(member):];
                for (const std::any& data : extra_data)
                {
                    if (const Member* casted = std::any_cast<Member>(&data))
                    {
                        result->[:member:] = *casted;
                        break;
                    }
                }
            }

            return result;
        }
    };
}
