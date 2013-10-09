/* cvector.h
 *
 * ease computation on periodic meshes of size (N=2^b)^R
 */

#pragma once

#include "mvector.hh"
#include "misc.hh"

#include <iostream>

namespace System
{
	inline unsigned dfloor(double v)
	{ return static_cast<unsigned>((v < 0 ? v - 1 : v)); }

	inline unsigned dround(double v)
	{ return dfloor(v + 0.5); }

	class InverseLog: public std::vector<size_t>
	{
		unsigned bits;
		
		public:
			InverseLog(unsigned b);
			size_t operator()(size_t i) const;
	};

	class ReverseBitorder: public std::vector<size_t>
	{
		unsigned bits;
		public:
			ReverseBitorder(unsigned b);
			size_t operator()(size_t i) const;
	};

	template <unsigned R>
	class cVector
	{
		unsigned b;
		size_t d_mask[R];

		InverseLog 		ilg;
		ReverseBitorder	rev;

		public:
			/*class C: public std::iterator<std::forward_iterator_tag, C>
			{
				cVector const   &v;
				size_t 			r;

				public:
					C(cVector const &v_, size_t r_):
						v(v_), r(r_) {}

					inline C operator+(C const &o) const
					{ return C(v, v.add(r, o.r)); }

					inline C operator-(C const &o) const
					{ return C(v, v.sub(r, o.r)); }

					inline C operator*(unsigned s) const
					{ return C(v, v.mul(r, s)); }

					inline C &operator++()
					{ ++r; return *this; }

					inline C &operator--()
					{ --r; return *this; }

					inline C operator<<(cVector const &w) const
					{ return C(w, v.promote(r, w.bits())); }

					inline operator size_t() const
					{ return r; }

					template <typename T>
					inline mVector<T, R> vec() const
					{
						mVector<T, R> a;
						for (unsigned i = 0; i < R; ++i)
							a[i] = (*this)[i];
						return a;
					}

					inline mVector<int, R> ivec() const
					{
						mVector<int, R> a;
						for (unsigned i = 0; i < R; ++i)
							a[i] = (*this)[i];
						return a;
					}

					inline mVector<double, R> dvec() const
					{
						mVector<double, R> a;
						for (unsigned i = 0; i < R; ++i)
							a[i] = (*this)[i];
						return a;
					}

					inline bool operator==(C const &o) const
					{ return r == o.r; }

					inline bool operator!=(C const &o) const
					{ return r != o.r; }

					inline size_t operator[](unsigned i) const
					{ return v.idx(r, i) >> (i * v.bits()); }

					// behave like pseudo-iterator	
					inline C &operator*() { return *this; }
			};*/

			//std::vector<C> dx, dx2, sq;
			std::vector<size_t> dx_i, dx2_i, sq_i;

			cVector(unsigned bits):
				b(bits), ilg(bits), rev(bits) 
			{
				for (unsigned i = 0; i < R; ++i)
				{
					d_mask[i] = calc_mask(i);
					//dx.push_back(C(*this, unit_vector(i)));
					dx_i.push_back(unit_vector(i));
					//dx2.push_back(C(*this, 2*unit_vector(i)));
					dx2_i.push_back(2 * unit_vector(i));
				}

				if (bits > 1)
				{
					cVector unit_cell(1);
					for (size_t i = 0; i < (1 << R); ++i)
					{
						sq_i.push_back(unit_cell.promote(i, bits));
						//sq.push_back(dq << *this);
					}
				}
			}
/*
			C begin() const
			{
				return C(*this, 0);
			}

			C end() const
			{
				return C(*this, size_t(1) << (b * R));
			}

			C operator[](size_t i) const
			{
				return C(*this, i);
			}
*/

			inline
			size_t loc(mVector<int, R> const &a) const
			{
				size_t n = 0;
				for (unsigned i = 0; i < R; ++i)
					n |= idx(a[i] << (i * b), i);
				return n;
			}

			template <typename T>
			size_t operator()(mVector<T, R> const &a) const
			{
				size_t n = 0;
				for (unsigned i = 0; i < R; ++i)
					n |= idx((unsigned)(a[i]) << (i * b), i);
				return n;
			}

			inline
			size_t unit_vector(unsigned i) const
			{
				return size_t(1) << (b * i);
			}

			inline
			unsigned bits() const 
			{ 
				return b; 
			}

			size_t size() const
			{
				return size_t(1) << (b * R);
			}

			unsigned extent() const
			{
				return 1 << b;
			}

			inline
			size_t calc_mask(unsigned i) const
			{
				return ((size_t(1) << b) - 1) << (b * i);
			}

			inline
			size_t mask(unsigned i) const
			{
				return d_mask[i];
			}

			inline
			size_t idx(size_t r, unsigned i) const
			{
				return r & mask(i);
			}

			inline
			size_t i(size_t r, unsigned i) const
			{
				return (r & mask(i)) >> (b * i);
			}

			inline
			mVector<double,R> dvec(size_t r) const
			{
				mVector<double,R> a;
				for (unsigned j = 0; j < R; ++j)
					a[j] = i(r, j);
				return a;
			}

			inline
			size_t promote(size_t r, unsigned bn) const
			{
				size_t n = 0;
				for (unsigned i = 0; i < R; ++i)
					n |= idx(r, i) << (bn - b) * i;
				return n;
			}

			inline 
			size_t add(size_t r, size_t s) const
			{
				size_t n = 0;
				for (unsigned i = 0; i < R; ++i)
					n |= idx(idx(r, i) + idx(s, i), i);
				return n;
			}

			inline
			size_t sub(size_t r, size_t s) const
			{
				size_t n = 0;
				for (unsigned i = 0; i < R; ++i)
					n |= idx(idx(r, i) - idx(s, i), i);
				return n;
			}

			inline
			size_t mul(size_t r, unsigned s) const
			{
				size_t n = 0;
				for (unsigned i = 0; i < R; ++i)
					n |= idx(idx(r, i) * s, i);
				return n;
			}

			inline size_t neg(size_t r) const
			{
				size_t n = 0;
				for (unsigned i = 0; i < R; ++i)
					n |= idx(0 - idx(r, i), i);
				return n;
			}

			inline size_t reverse(size_t r) const
			{
				size_t n = 0;
				for (unsigned j = 0; j < R; ++j)
					n |= rev[i(r, j)] << (b * j);
				return n;
			}

			inline size_t invlog(size_t r) const
			{
				size_t n = 0;
				for (unsigned j = 0; j < R; ++j)
					n |= ilg[i(r, j)] << (b * j);
				return n;
			}
	};
}

// vim:ts=4:sw=4:tw=80
