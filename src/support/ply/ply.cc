#include "ply.hh"

using namespace PLY;

PLY::PLY::PLY(Format format)
{
    header_.format = format;
}

PLY::PLY::PLY(std::string const &file_name)
{
    std::ifstream fi(file_name, std::ios::binary);
    fi >> header_;

    for (auto const &element : header_)
    {
        data_[element.name] = RecordArray(element.spec, fi, element.size);
    }
}

std::ostream &operator<<(std::ostream &out, PLY::PLY const &ply)
{
    out << ply.header();
    for (Element const &element : ply.header())
    {
        if (ply.format() == BINARY)
        {
            out.write(ply[element.name].data(), ply[element.name].size());
        } else {
            out << ply[element.name];
        }
    }
    return out;
}
