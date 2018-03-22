#pragma once
#include <iostream>
#include <iterator>
#include <utility>
#include <tuple>
#include <experimental/tuple>

namespace System
{
    /*! \brief get the head of a range
     * \param r a ranged object (having begin and end methods)
     * \return reference to first element of the range.
     */
    template <typename R>
    auto head(R const &r) -> decltype(*r.begin())
    {
        return *r.begin();
    }

    template <typename T>
    class Tail
    {
        public:
            typedef typename T::value_type value_type;
            typedef typename T::const_iterator const_iterator;

            Tail(T const &a):
                b(a.begin()), e(a.end())
            {
                ++b;
            }

            const_iterator begin() const { return b; }
            const_iterator end() const { return e; }
            const_iterator cbegin() const { return b; }
            const_iterator cend() const { return e; }

        private:
            const_iterator b, e;
    };

    /*! \brief get the tail of a range
     * \param r a range object (having begin and end methods).
     * \return a Tail object combining begin()++ and end().
     */
    template <typename T>
    Tail<T> tail(T const &a) { return Tail<T>(a); }
   

    template <typename T>
    class Reversed
    {
        T const &p;
        public:
            using const_iterator = typename T::const_reverse_iterator;
            using iterator = typename T::reverse_iterator;
            
            Reversed(T const &p_): p(p_) {}
            const_iterator begin() const { return p.rbegin(); }
            const_iterator end() const { return p.rend(); }  
    };
    
    template <typename T>
    Reversed<T> reversed(T const &p) { return Reversed<T>(p); }


    template <typename R, typename I>
    void copy(R const &in, I &out)
    {
        std::copy(in.begin(), in.end(), std::back_inserter(out));
    }

    /*! Defines a small wrapper class for std::string to read lines of input.
     */
    class InputLine
    {
        std::string data;

        public:
            friend std::istream &operator>>(std::istream &in, InputLine &l) 
            {
                std::getline(in, l.data);
                return in;
            }

            operator std::string() const
            {
                return data;
            }
    };


    /*! Captures std::istream_iterator<InputLine> in a range (giving
     * begin and end methods) allowing for easy iteration of lines.
     */
    template <typename T = char>
    class ReadFile
    {
        std::istream &in;
        
        public:
            using iterator = std::istream_iterator<T>;

            ReadFile(std::istream &in): in(in) {}
            iterator begin() { return iterator(in); }
            iterator end() { return iterator(); }
    };

    /*! Returns a ReadLines range, iterating over lines in a file.
     */
    inline ReadFile<InputLine> read_lines(std::istream &in)
    {
        return ReadFile<InputLine>(in);
    }

    inline ReadFile<char> read_file(std::istream &in)
    {
        return ReadFile<char>(in);
    }

    template <typename Range, typename Out, typename Fn>
    void transmogrify(Range const &range, Out out, Fn &&fn)
    {
        for (auto i = std::begin(range); i != std::end(range); ++i)
            *out++ = std::experimental::apply(std::forward<Fn>(fn), *i);
    }

}

