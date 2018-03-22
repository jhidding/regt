#pragma once
#include "base.hh"
#include "record-array.hh"

namespace PLY
{
    struct Element
    {
        std::string name;
        RecordSpec  spec;
        size_t      size;

        Element(std::string const &name, RecordSpec const &spec, size_t size):
            name(name), spec(spec), size(size) {}

        template <typename ...Args>
        Element(std::string const &name, Field const &field, Args &&... rest):
            name(name), size(0)
        {
            add_properties(field, std::forward<Args>(rest)...);
        }

    private:
        void add_properties(Field const &field)
        { 
            spec.push_back(field);
        }
        
        template <typename ...Args>
        void add_properties(Field const &field, Args &&... rest)
        {
            spec.push_back(field);
            add_properties(std::forward<Args>(rest)...);
        }
    };

    class Header: public std::vector<Element>
    {
        public:
            Format format;
            std::vector<std::string> comments;

            template <typename ...Args>
            void add_element(std::string name, Args &&... properties)
            {
                push_back(Element(name, std::forward<Args>(properties)...));
            }
    };
}
