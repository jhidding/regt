#include "record-print.hh"
#include "record-read.hh"

using namespace PLY;

inline char const *print_field(std::ostream &out, TYPE_ID type, char const *data)
{
    switch (type)
    {
        case T_CHAR:    out << static_cast<int>(cast<int8_t  >(data));
                        return data + 1;
        case T_UCHAR:   out << static_cast<unsigned>(cast<uint8_t >(data));
                        return data + 1;
        case T_SHORT:   out << cast<int16_t >(data);
                        return data + 2;
        case T_USHORT:  out << cast<uint16_t>(data);
                        return data + 2;
        case T_INT:     out << cast<int32_t >(data);
                        return data + 4;
        case T_UINT:    out << cast<uint32_t>(data);
                        return data + 4;
        case T_FLOAT:   out << cast<float   >(data);
                        return data + 4;
        case T_DOUBLE:  out << cast<double  >(data);
                        return data + 8;
    }

    throw Exception("Error reading record field: unknown data type.");
}

inline char const *print_record(std::ostream &out, RecordSpec const &spec, char const *data)
{
    for (Field const &f : spec)
    {
        if (f.is_list)
        {
            size_t len;
            data = read_field(f.length_type, data, len);
            out << len << " ";
            for (unsigned i = 0; i < len; ++i)
            {
                data = print_field(out, f.type, data);
                out << " ";
            }
        }
        else
        {
            data = print_field(out, f.type, data);
            out << " ";
        }
    }

    return data;
}

std::ostream &PLY::operator<<(std::ostream &out, RecordArray const &d)
{
    char const *data = d.data();
    for (size_t i = 0; i < d.size(); ++i)
    {
        data = print_record(out, d.spec, data);
        out << "\n";
    }
    return out;
}

