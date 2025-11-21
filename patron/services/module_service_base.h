#pragma once
#include "patron/commands/type_reader.h"
#include "patron/utils/concepts.h"
#include "patron/utils/strings.h"
#include <functional>
#include <typeindex>
#include <unordered_map>

namespace patron
{
    struct module_service_config
    {
        bool case_sensitive_lookup{};
        char command_prefix = '!';
        char separator_char = ' ';
        bool throw_exceptions{};
    };

    class module_service_base
    {
    public:
        explicit module_service_base(module_service_config config = {})
            : m_config(std::move(config)) {}

        const module_service_config& config() const { return m_config; }

        template<typename T>
        std::unique_ptr<type_reader_base<T>> create_type_reader() const
        {
            if (auto it = m_type_reader_factories.find(typeid(T)); it != m_type_reader_factories.end())
                return std::unique_ptr<type_reader_base<T>>(static_cast<type_reader_base<T>*>(it->second()));
            return nullptr;
        }

        template<typename T>
        void register_extra_data(T&& data = {})
        {
            m_extra_data.emplace_back(std::forward<T>(data));
        }

        template<utility::specialization_of<type_reader> T>
        void register_type_reader()
        {
            const std::type_info& info = typeid(typename T::value_type);
            auto [_, success] = m_type_reader_factories.try_emplace(info, [this] -> void* { return T::create(m_extra_data); });
            if (!success)
                throw std::logic_error("A type reader has already been registered for " + utility::demangle(info.name()));
        }
    private:
        module_service_config m_config;
        std::vector<std::any> m_extra_data;
        std::unordered_map<std::type_index, std::function<void*()>> m_type_reader_factories;
    };
}
