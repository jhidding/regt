#include "record-base.hh"
#include "record-write.hh"
#include <utility>
#include <tuple>
#include <iostream>

namespace PLY
{
    template <typename U, typename T, typename Input>
    inline T load_bytes(Input &data)
    {
        U tgt;
        data.read(reinterpret_cast<char *>(&tgt), sizeof(U));
        return static_cast<T>(tgt);
    }

    template <typename T, typename Input>
    inline T load_field(Input &data, TYPE_ID type)
    {
        switch (type)
        {
            case T_CHAR:    return load_bytes<int8_t,   T>(data);
            case T_UCHAR:   return load_bytes<uint8_t,  T>(data);
            case T_SHORT:   return load_bytes<int16_t,  T>(data);
            case T_USHORT:  return load_bytes<uint16_t, T>(data);
            case T_INT:     return load_bytes<int32_t,  T>(data);
            case T_UINT:    return load_bytes<uint32_t, T>(data);
            case T_FLOAT:   return load_bytes<float,    T>(data);
            case T_DOUBLE:  return load_bytes<double,   T>(data);
        }

        throw Exception("Error reading record field: unknown data type.");
    }

    using bytes = std::istreambuf_iterator<char>;

    template <typename Input, typename OutputIterator>
    inline OutputIterator load_list(
        Input &data,
        OutputIterator tgt,
        Field const &f)
    {
        size_t len = load_field<size_t>(data, f.length_type);
        tgt = write_field(f.length_type, tgt, len);
        std::vector<char> buf(len * Type_map[f.type].size);
        data.read(buf.data(), buf.size());
        return std::copy(buf.begin(), buf.end(), tgt);
    }

    template <typename Input, typename OutputIterator>
    inline OutputIterator load_record(
        Input &data,
        OutputIterator tgt,
        RecordSpec const &spec)
    {
        for (Field const &f : spec)
        {
            if (f.is_list)
                tgt = load_list(data, tgt, f);
            else
            {
                std::vector<char> buf(Type_map[f.type].size);
                data.read(buf.data(), buf.size());
                tgt = std::copy(buf.begin(), buf.end(), tgt);
            }
        }
        return tgt;
    }

    template <typename Input, typename OutputIterator>
    inline OutputIterator load_array(
        Input &data, 
        OutputIterator tgt,
        RecordSpec const &spec, size_t n)
    {
        if (has_list(spec))
        {
            for (size_t i = 0; i < n; ++i)
                tgt = load_record(data, tgt, spec);
            return tgt;
        }
        else
        {
            size_t r_size = record_size(spec);
            std::vector<char> buf(r_size * n);
            data.read(buf.data(), buf.size());
            return std::copy(buf.begin(), buf.end(), tgt);
        }
    }
}

