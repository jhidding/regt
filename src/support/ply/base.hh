#pragma once
#include <string>
#include <memory>
#include <stdint.h>
#include <stdexcept>
#include <map>
#include <vector>
#include <experimental/optional>

namespace PLY
{
    using std::experimental::optional;
    using std::experimental::make_optional;
    using std::experimental::nullopt;
    using std::experimental::in_place;

    class Exception: public std::exception
    {
        std::string msg;

        public:
            Exception(std::string const &msg_): msg(msg_) {}
            char const *what() const throw () { return msg.c_str(); }
            ~Exception() throw () {}
    };

    enum Format { ASCII, BINARY };

    enum TYPE_ID { 
        T_CHAR, T_UCHAR, T_SHORT, T_USHORT, 
        T_INT,  T_UINT,  T_FLOAT, T_DOUBLE  };

    template <typename T>
    struct Type {
        static TYPE_ID     id;
        static std::string name;
    };

    struct Type_info
    {
        std::string name;
        unsigned    size;
    };

    extern std::map<TYPE_ID, Type_info> Type_map;
    extern std::map<std::string, TYPE_ID> Type_id_map;
}

