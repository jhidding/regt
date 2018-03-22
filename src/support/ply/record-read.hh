#pragma once
#include "record-base.hh"
#include <utility>
#include <tuple>

namespace PLY
{
    template <typename T>
    inline char const *read_field(TYPE_ID type, char const *data, T &tgt)
    {
        switch (type)
        {
            case T_CHAR:    tgt = cast<T, int8_t  >(data);
                            return data + 1;
            case T_UCHAR:   tgt = cast<T, uint8_t >(data);
                            return data + 1;
            case T_SHORT:   tgt = cast<T, int16_t >(data);
                            return data + 2;
            case T_USHORT:  tgt = cast<T, uint16_t>(data);
                            return data + 2;
            case T_INT:     tgt = cast<T, int32_t >(data);
                            return data + 4;
            case T_UINT:    tgt = cast<T, uint32_t>(data);
                            return data + 4;
            case T_FLOAT:   tgt = cast<T, float   >(data);
                            return data + 4;
            case T_DOUBLE:  tgt = cast<T, double  >(data);
                            return data + 8;
        }

        throw Exception("Error reading record field: unknown data type.");
    }

    template <typename T>
    inline char const *read_list(Field const &f, char const *data, T &v)
    {
        throw Exception("A list needs to be read to a std::vector.");
    }

    template <typename T>
    inline char const *read_list(Field const &f, char const *data, std::vector<T> &v)
    {
        size_t size;
        data = read_field(f.length_type, data, size);

        v.resize(size);

        for (size_t i = 0; i < size; ++i)
            data = read_field(f.type, data, v[i]);

        return data;
    }

    template <typename T>
    inline char const *read_record(
        RecordSpec const &spec, char const *data, T &tgt, std::index_sequence<>)
    {
        return data;
    }

    template <typename T, size_t i, size_t ...is>
    inline char const *read_record(
        RecordSpec const &spec, char const *data, T &tgt, std::index_sequence<i,is...>)
    {
        if (spec[i].is_list)
            return read_list(spec[i], data, std::get<i>(tgt));
        else
            return read_record(spec, 
                read_field(spec[i].type, data, std::get<i>(tgt)), 
                tgt, std::index_sequence<is...>());
    }

    template <typename T>
    inline char const *read_record(
        RecordSpec const &spec, char const *data, T &tgt)
    {
        return read_record(spec, data, tgt, 
            std::make_index_sequence<std::tuple_size<T>::value>());
    }
}

