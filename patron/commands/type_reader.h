#pragma once

namespace patron
{
    struct type_reader_base
    {
        virtual ~type_reader_base() = default;
    };

    template<typename T>
    class type_reader : public type_reader_base
    {
    public:
        using value_type = T;
    };
}
