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

void PLY::PLY::save(std::string const &filename) const
{
    std::ofstream out(filename);
    
    out << header();
    for (Element const &element : header())
    {
        if (format() == BINARY)
        {
            out.write((*this)[element.name].data(), (*this)[element.name].byte_size());
        } else {
            out << (*this)[element.name];
        }
    }

    out.close();
}
