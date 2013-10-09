#pragma once
#include <string>
#include <iterator>
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <memory>
#include <functional>
#include <sstream>
#include <complex>

namespace System
{
	// types and pointers {{{1
	typedef std::complex<double> complex64;

	template <typename T>
	using ptr = std::shared_ptr<T>;

	typedef std::string Exception;

	template <typename T, typename ...Args>
	ptr<T> make_ptr(Args &&...args)
	{ return std::make_shared<T>(std::forward<Args>(args)...); }

	typedef std::function<void (int, char **)> Command;
	// }}}1

	// IO {{{1
	template <typename T>
	void read_block(std::istream &f, std::vector<T> &v)
	{
		uint64_t bs1, bs2;	
		f.read(reinterpret_cast<char *>(&bs1), sizeof(uint64_t));
		if (f.fail())
		{
			throw "Could not read from file";
		}

		v.resize(bs1 / sizeof(T));
		f.read(reinterpret_cast<char *>(v.data()), bs1);
		f.read(reinterpret_cast<char *>(&bs2), sizeof(uint64_t));

		if (bs1 != bs2)
			throw "The block seems to be corrupted";
	}

	template <typename T>
	void write_block(std::ostream &f, std::vector<T> const &v)
	{
		uint64_t byte_size = v.size() * sizeof(T);
		f.write(reinterpret_cast<char *>(&byte_size), sizeof(uint64_t));
		f.write(reinterpret_cast<char const *>(v.data()), byte_size);
		f.write(reinterpret_cast<char *>(&byte_size), sizeof(uint64_t));
	}

	inline void skip_block(std::istream &f)
	{
		uint64_t bs1, bs2;

		f.read(reinterpret_cast<char *>(&bs1), sizeof(uint64_t));
		if (f.fail())
		{
			throw "Could not read from file";
		}

		f.seekg(bs1, std::ios::cur);

		f.read(reinterpret_cast<char *>(&bs2), sizeof(uint64_t));
		if (bs1 != bs2)
			throw "The block seems to be corrupted";
	}

	template <typename T>
	T from_string(std::string const &s)
	{
		std::istringstream in(s);
		T value;
		in >> value;
		return value;
	}

	template <typename T>
	std::string to_string(T const &value)
	{
		std::ostringstream out;
		out << value;
		return out.str();
	}
	// }}}1

	// numerics {{{1
	// range {{{2
	/*! 
	 * generates a range of integers
	 */
	class Range
	{
		unsigned k;

		public:
			Range(unsigned k_): k(k_) {}
			
			class const_iterator: 
				public std::iterator<std::forward_iterator_tag, unsigned>
			{
				unsigned i;
				public:
					const_iterator(unsigned i_): i(i_) {}
					const_iterator &operator++() { ++i; return *this; }
					bool operator==(const_iterator const &o) const { return i == o.i; }
					bool operator!=(const_iterator const &o) const { return i != o.i; }
					unsigned operator*() const { return i; }
			};

			const_iterator begin() const { return const_iterator(0); }
			const_iterator end() const { return const_iterator(k); }
	};
	// }}}2
	
	// modulus {{{2
	/*!
	 * calculates a mod b, by the mathematical definition:
	 * the C operator % does not give the right answer if a < 0
	 */
	inline int modulus(int a, int b)
	{
		if (a < 0) return b + (a %  b);
		else return (a % b);
	}

	/*!
	 * calculates the a mod b, where a and b are double floats.
	 */
	inline double modulus(double a, double b)
	{
		if (a < 0) return a - static_cast<int>(a / b - 1) * b;
		else return a - static_cast<int>(a / b) * b;
	}
	// }}}2

	// product {{{2
	template <typename T>
	typename T::value_type product(T const &A)
	{
		return std::accumulate(A.begin(), A.end(), (typename T::value_type)(1), 
				std::multiplies<typename T::value_type>());
	}	
	// }}}2

	// gaussian noise generator {{{2
	/*!
	 * generates gaussian white noise with a sigma = 1 and mean = 0
	 */
	inline std::function<double ()> Gaussian_white_noise(unsigned long seed)
	{
		std::shared_ptr<std::mt19937> 
			random(new std::mt19937(seed));
		std::shared_ptr<std::normal_distribution<double>> 
			normal(new std::normal_distribution<double>);

		return [random, normal] () -> double 
			{ return (*normal)(*random); };
	}
	// }}}2
	// }}}1

	// ranges {{{1
	template <typename A, typename F>
	void generate(A const &a, F const &f)
	{ std::generate(a.begin(), a.end(), f); }

	template <typename A, typename B>
	void copy(A const &a, B &b) 
	{ std::copy(a.begin(), a.end(), b.begin()); }

	template <typename A, typename B>
	void fill(A const &a, B b)
	{ std::fill(a.begin(), a.end(), b); }

	template <typename A, typename B, typename Fun>
	void transform(A const &a, B &b, Fun f)
	{ std::transform(a.begin(), a.end(), b.begin(), f); }

	template <typename A, typename B, typename C, typename Fun>
	void transform(A const &a, B const &b, C &c, Fun f)
	{ std::transform(a.begin(), a.end(), b.begin(), c.begin(), f); }

	template <typename A>
	typename A::const_iterator max_element(A const &a)
	{ return std::max_element(a.begin(), a.end()); }

	template <typename A>
	typename A::const_iterator min_element(A const &a)
	{ return std::min_element(a.begin(), a.end()); }

	template <typename T>
	class Assoc
	{
		typedef typename T::const_iterator range_iterator;
		T range;

		public:
			typedef std::pair<typename T::arg_type, typename T::value_type> value_type;

			class const_iterator:
				public std::iterator<std::forward_iterator_tag, value_type>,
				public range_iterator
			{
				public:
					const_iterator(range_iterator const &i): range_iterator(i) {};
					value_type operator*() const 
					{ return value_type(range_iterator::arg(), range_iterator::operator*()); }
			};

			const_iterator begin() const { return const_iterator(range.begin()); }
			const_iterator end() const { return const_iterator(range.end()); }
			const_iterator cbegin() const { return const_iterator(range.begin()); }
			const_iterator cend() const { return const_iterator(range.end()); }
	};

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

	template <typename T>
	Tail<T> tail(T const &a) { return Tail<T>(a); }
	// }}}1

	template <typename R>
	std::string join(R const &r, std::string const &d)
	{
		std::string a = head(r);
		for (auto b : tail(r))
			a += d + b;

		return a;
	}
}

// vim:ts=4:sw=4:tw=80:fdm=marker
