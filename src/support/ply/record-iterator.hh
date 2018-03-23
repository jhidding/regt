#pragma once
#include "record-base.hh"
#include "record-read.hh"
#include <cstddef>
#include <iterator>

namespace PLY
{
    template <typename ByteIterator, typename T>
    class RecordOutputIterator: public std::iterator<std::output_iterator_tag, T>
    {
        RecordSpec const &spec;
        ByteIterator data;

        public:
            RecordOutputIterator &operator++()
                { return *this; }

            RecordOutputIterator &operator*()
                { return *this; }

            void operator=(T const &value)
                { write_record(spec, data, value); }
    };

    template <typename T>
    class RecordInputIterator: public std::iterator<std::input_iterator_tag, T>
    {
        RecordSpec const &spec;
        char const *data;
        mutable char const *next;
        mutable T record;

        public:
            RecordInputIterator(RecordSpec const &spec, char const *data):
                spec(spec), data(data), next(data)
            {}

            RecordInputIterator &operator++()
            {
                data = next;
                return *this;
            }

            T const &operator*() const
            {
                next = read_record(spec, data, record);
                return record;
            }

            bool operator!=(RecordInputIterator const &other) const
            {
                return data != other.data;
            }
    };

    class RecordCountIterator: public std::iterator<std::forward_iterator_tag, size_t>
    {
        RecordSpec const &spec;
        char const *data;
        size_t counter;

        public:
            RecordCountIterator(RecordSpec const &spec, char const *data):
                spec(spec), data(data), counter(0)
            {}

            RecordCountIterator &operator++()
            {
                data = skip_record(spec, data);
                ++counter;
                return *this;
            }

            size_t operator*() const
            {
                return counter;
            }

            bool operator!=(RecordCountIterator const &other) const
            {
                return data != other.data;
            }
    };
}

