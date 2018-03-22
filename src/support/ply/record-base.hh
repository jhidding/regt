#pragma once
#include "base.hh"
#include <vector>
#include <algorithm>
#include <iostream>

namespace PLY
{
    struct Field
    {
        TYPE_ID type, length_type;
        bool is_list;
        std::string name;

        Field(TYPE_ID type, std::string const &name):
            type(type), is_list(false), name(name) {}

        Field(TYPE_ID type, TYPE_ID length_type, std::string const &name):
            type(type), length_type(length_type), is_list(true), name(name) {}
    };

    template <typename T>
    Field property(std::string const &name)
    {
        return Field(Type<T>::id, name);
    }

    template <typename T, typename U>
    Field list_property(std::string const &name)
    {
        return Field(Type<T>::id, Type<U>::id, name);
    }

    using RecordSpec = std::vector<Field>;

    inline bool has_list(RecordSpec const &r)
    {
        return std::any_of(r.begin(), r.end(),
            [] (Field const &f) { return f.is_list; });
    }

    inline size_t record_size(RecordSpec const &r)
    {
        size_t a = 0;
        for (Field const &f : r)
            a += Type_map[f.type].size;
        return a;
    }

    template <typename T, typename U = T>
    inline T cast(char const *d)
    {
        return static_cast<T>(*reinterpret_cast<U const *>(d));
    }

    template <typename U, typename T, typename Iterator>
    inline T from_bytes(Iterator &data)
    {
        U tgt;
        std::copy_n(data, sizeof(U), reinterpret_cast<char *>(&tgt));
        return static_cast<T>(tgt);
    }

    template <typename U, typename T>
    inline std::array<char, sizeof(U)> to_bytes(T const &value)
    {
        std::array<char, sizeof(U)> result;
        *reinterpret_cast<U *>(&result) = static_cast<U>(value);
        return result;
    }
}

