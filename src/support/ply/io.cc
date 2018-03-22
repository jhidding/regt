#include "io.hh"

using namespace PLY;


std::ostream &PLY::operator<<(std::ostream &out, Field const &f)
{
    if (f.is_list)
        return out << "property list " << Type_map[f.length_type].name 
                   << " " << Type_map[f.type].name 
                   << " " << f.name << "\n";
    else
        return out << "property " << Type_map[f.type].name 
                   << " " << f.name << "\n";
}


std::ostream &PLY::operator<<(std::ostream &out, RecordSpec const &spec)
{
    for (Field const &f : spec)
        out << f;
    return out;
}

std::ostream &PLY::operator<<(std::ostream &out, Element const &e)
{
    out << "element " << e.name << " " << e.size << "\n";
    return out << e.spec;
}

std::ostream &PLY::operator<<(std::ostream &out, Header const &h)
{
    out << "ply\n"
        << "format " 
        << (h.format == ASCII ? "ascii" : "binary_little_endian")
        << " 1.0\n";

    for (std::string const &c : h.comments)
        out << "comment " << c << "\n";

    for (Element const &e : h)
        out << e;

    return out << "end_header\n";
}

