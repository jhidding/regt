#include "record-base.hh"
#include <utility>
#include <tuple>

namespace PLY
{
    template <typename Iterator, typename ByteArray>
    inline Iterator write_bytes(Iterator data, ByteArray const &bytes)
    {
        for (size_t i = 0; i < std::tuple_size<ByteArray>::value; ++i)
            *data++ = bytes[i];
        return data;
    }

    template <typename Iterator, typename T>
    inline Iterator write_field(TYPE_ID type, Iterator data, T const &value)
    {
        switch (type)
        {
            case T_CHAR:    return write_bytes(data, to_bytes<int8_t,   T>(value));
            case T_UCHAR:   return write_bytes(data, to_bytes<uint8_t,  T>(value));
            case T_SHORT:   return write_bytes(data, to_bytes<int16_t,  T>(value));
            case T_USHORT:  return write_bytes(data, to_bytes<uint16_t, T>(value));
            case T_INT:     return write_bytes(data, to_bytes<int32_t,  T>(value));
            case T_UINT:    return write_bytes(data, to_bytes<uint32_t, T>(value));
            case T_FLOAT:   return write_bytes(data, to_bytes<float,    T>(value));
            case T_DOUBLE:  return write_bytes(data, to_bytes<double,   T>(value));
        }

        throw Exception("Unknown data type");
    }

    template <typename Iterator, typename Vector>
    inline Iterator write_list(Field const &f, Iterator data, Vector const &v)
    {
        write_field(f.length_type, data, v.size());
        for (auto const &item : v)
            write_field(f.type, data, item);
        return data;
    }

    template <typename Iterator, typename Tuple, unsigned i, unsigned ...is>
    inline Iterator write_record(RecordSpec const &spec, Iterator data, Tuple const &t,
        std::index_sequence<>)
    {
        return data;
    }

    template <typename Iterator, typename Tuple, unsigned i, unsigned ...is>
    inline Iterator write_record(RecordSpec const &spec, Iterator data, Tuple const &t,
        std::index_sequence<i, is...>)
    {
        if (spec[i].is_list)
            return write_list(spec[i], data, std::get<i>(t));
        else
            return write_field(spec[i].type, data, std::get<i>(t));
    }

    template <typename Iterator, typename Tuple>
    inline Iterator write_record(RecordSpec const &spec, Iterator data, Tuple const &t)
    {
        return write_record(spec, data, t, 
            std::make_index_sequence<std::tuple_size<Tuple>::value>());
    }
}

