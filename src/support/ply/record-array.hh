#pragma once
#include "record-base.hh"
#include "record-iterator.hh"
#include "record-io.hh"
#include <tuple>
#include <vector>

namespace PLY
{
    template <typename T>
    class RecordArrayView
    {
        RecordSpec const &spec;
        char const *data;
        size_t size;

        public:
            RecordArrayView(RecordSpec const &spec, char const *data, size_t size):
                spec(spec), data(data), size(size)
            {
                if (std::tuple_size<T>::value != spec.size())
                {
                    throw Exception("Trying to iterate a RecordArray with wrong tuple or array.");
                }
            }

            RecordInputIterator<T> begin() const
            { 
                return RecordInputIterator<T>(spec, data);
            }

            RecordInputIterator<T> end() const
            {
                return RecordInputIterator<T>(spec, data + size);
            }
    };

    class RecordArray
    {
        std::vector<char> data_;
        size_t size_;

        public:
            RecordSpec spec;

            RecordArray() {}

            RecordArray(RecordSpec const &_spec):
                size_(0), spec(_spec)
            {}

            template <typename Iterator>
            RecordArray(RecordSpec const &spec, Iterator &input, size_t n):
                size_(n), spec(spec)
            {
                auto output = std::back_inserter(data_);
                load_array(input, output, spec, n);
            }
            
            template <typename T>
            RecordArray &field(std::string const &name)
            {
                spec.emplace_back(Type<T>::id, name);
                return *this;
            }

            template <typename S, typename T>
            RecordArray &list(std::string const &name)
            {
                spec.emplace_back(Type<T>::id, Type<S>::id, name);
                return *this;
            }

            size_t size() const
            {
                return size_;
            }

            char const *data() const
            {
                return data_.data();
            }

            template <typename ...Args>
            void push_back(Args &&... args)
            {
                auto output = std::back_inserter(data_);
                write_back(output, 0, std::forward<Args>(args)...);
                ++size_;
            }

            template <typename ...Args>
            RecordArrayView<std::tuple<Args...>> as() const
            {
                return RecordArrayView<std::tuple<Args...>>(
                    spec, data(), data_.size());
            }

            template <typename T, size_t N>
            RecordArrayView<std::array<T, N>> as_array() const
            {
                return RecordArrayView<std::array<T, N>>(
                    spec, data(), data_.size());
            }
            
        private:
            template <typename Output, typename Item>
            void write_item(Output &output, int idx, std::vector<Item> const &item)
            {
                if (not spec[idx].is_list)
                    throw Exception("std::vector of values given, but not expected.");
                write_list(spec[idx], output, item);
            }

            template <typename Output, typename Item>
            void write_item(Output &output, int idx, Item item)
            {
                if (spec[idx].is_list)
                    throw Exception("Single value given, but std::vector expected.");
                write_field(spec[idx].type, output, item);
            }
            
            template <typename Output, typename Last>
            void write_back(Output &output, int idx, Last last)
            {
                write_item(output, idx, last);
            }

            template <typename Output, typename First, typename ...Rest>
            void write_back(Output &output, int idx, First first, Rest &&... rest)
            {
                write_item(output, idx, first);
                write_back(output, idx + 1, std::forward<Rest>(rest)...);
            }
    };
}

