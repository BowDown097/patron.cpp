#pragma once
#include "patron/results/type_reader_result.h"
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

        constexpr const T* operator->() const noexcept { return std::addressof(m_value); }
        constexpr T* operator->() noexcept { return std::addressof(m_value); }
        constexpr const T& operator*() const& noexcept { return m_value; }
        constexpr T& operator*() & noexcept { return m_value; }
        constexpr const T&& operator*() const&& noexcept { return std::move(m_value); }
        constexpr T&& operator*() && noexcept { return std::move(m_value); }

        constexpr const T& value() const& noexcept { return m_value; }
        constexpr T& value() & noexcept { return m_value; }
        constexpr const T&& value() const&& noexcept { return std::move(m_value); }
        constexpr T&& value() && noexcept { return std::move(m_value); }

        constexpr float weight() const { return m_weight; }
    private:
        T m_value;
        float m_weight;
    };

    struct type_reader_base
    {
        virtual ~type_reader_base() = default;
    };

    template<typename T, typename ContextType, typename EventType>
    struct type_reader_read
    {
        virtual type_reader_result read(ContextType* context, const EventType* event, const std::string& input) = 0;
    };

    template<typename T>
    struct type_reader_read<T, void, void>
    {
        virtual type_reader_result read(const std::string& input) = 0;
    };

    template<typename T, typename ContextType>
    struct type_reader_read<T, ContextType, void>
    {
        virtual type_reader_result read(ContextType* context, const std::string& input) = 0;
    };

    template<typename T, typename EventType>
    struct type_reader_read<T, void, EventType>
    {
        virtual type_reader_result read(const EventType* event, const std::string& input) = 0;
    };

    template<typename T, typename ContextType = void, typename EventType = void>
    class type_reader : public type_reader_base, public type_reader_read<T, ContextType, EventType>
    {
    public:
        using context_type = ContextType;
        using event_type = EventType;
        using value_type = T;

        const T& top_result() const
        {
            if (!has_result())
                throw std::logic_error("Tried to get top result from type reader with no results");

            const type_reader_value<T>* top_value{};
            for (const type_reader_value<T>& value : m_results)
                if (!top_value || top_value->weight() < value.weight())
                    top_value = &value;

            return top_value->value();
        }

        bool has_result() const { return !m_results.empty(); }
        explicit operator bool() const { return has_result(); }

        std::span<const type_reader_value<T>> results() const { return m_results; }
    protected:
        template<typename U = T> requires std::is_constructible_v<T, U>
        void add_result(U&& value, float weight = 1.0f) noexcept(std::is_nothrow_constructible_v<T, U>)
        {
            m_results.emplace_back(std::forward<U>(value), weight);
        }
    private:
        std::vector<type_reader_value<T>> m_results;
    };
}
