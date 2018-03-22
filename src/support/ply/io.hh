#pragma once
#include "base.hh"
#include "record-array.hh"
#include "header.hh"

namespace PLY
{
    extern std::ostream &operator<<(std::ostream &out, Field const &f);
    extern std::ostream &operator<<(std::ostream &out, RecordSpec const &spec);
    extern std::ostream &operator<<(std::ostream &out, Element const &e);
    extern std::ostream &operator<<(std::ostream &out, Header const &h);

    extern std::istream &operator>>(std::istream &fi, Header &h);  // throw (Exception);
} // namespace PLY

