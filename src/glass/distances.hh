#pragma once
#include <algorithm>
#include <numeric>
#include "../base/system.hh"
#include "../misc/kdtree.hh"

namespace Glass
{

using System::mVector;

template <unsigned R>
inline std::function<mVector<double,R> ()> 
	random_uniform_particles(unsigned long seed, double L)
{
	typedef mVector<double, R> Point;

	auto random  = System::make_ptr<std::mt19937>(seed);
	auto uniform = System::make_ptr<std::uniform_real_distribution<double>>(0, L);

	return [random, uniform] () -> Point
	{ 
		Point p;
		for (double &a : p) a = (*uniform)(*random);
		return p;
	};
}

inline double mod_L(double L, double x)
{
	if (x < -L/2) return x + L;
	if (x > L/2) return x - L;
	return x;
}

template <unsigned R>
inline mVector<double,R> dist(double L, mVector<double, R> const &A, mVector<double, R> const &B)
{
	mVector<double,R> d = (B - A);
	for (unsigned k = 0; k < R; ++k)
		d[k] = mod_L(L, d[k]);	
	return d;
}

template <unsigned R>
inline double dsqr(double L, mVector<double,R> const &A, mVector<double,R> const &B)
{
	return dist(L, A, B).sqr();
}

template <unsigned R>
class Distance_squared: public kdTree::Distance<mVector<double,R>, R>
{
	typedef mVector<double, R> Point;

	Point A;
	double L;

	public:
		Distance_squared(double L_, Point const &A_): A(A_), L(L_) {}

		virtual double operator()(Point const &B) const
		{
			double d = dsqr(L, A, B);
			if (d == 0) return 1e10;
			else return d;
		}

		virtual double operator()(kdTree::BoundingBox<Point, R> const &B) const
		{
			bool inside = true;
			double d = 0;

			for (unsigned k = 0; k < R; ++k)
			{
				if (A[k] < B.max_coord(k) and A[k] >= B.min_coord(k))
					continue;

				inside = false;
				d += std::min(pow(mod_L(L, A[k] - B.max_coord(k)), 2),
					      pow(mod_L(L, A[k] - B.min_coord(k)), 2));
			}

			return (inside ? -1 : d);
		};
};

template <unsigned R>
class Disc: public kdTree::Predicate<mVector<double, R>,R>
{
	typedef mVector<double, R> Point;

	Point A;
	double L, r;

	public:
		Disc(double L_, Point const &A_, double a_): 
			A(A_), L(L_), r(a_*a_) {}

		// if for one of the coordinates the reference point is in the same range
		// as the bounding box, that axis does not contribute to the miminum distance
		double min_distance(kdTree::BoundingBox<Point, R> const &B) const
		{
			bool inside = true;
			double d = 0;

			for (unsigned k = 0; k < R; ++k)
			{
				if (A[k] < B.max_coord(k) and A[k] >= B.min_coord(k))
					continue;

				inside = false;
				d += std::min(pow(mod_L(L, A[k] - B.max_coord(k)), 2),
					      pow(mod_L(L, A[k] - B.min_coord(k)), 2));
			}

			return (inside ? -1 : d);
		}

		virtual bool operator()(Point const &B) const
		{
			double D = dsqr(L, A, B);
			return D < r;
	       	}

		virtual bool operator()(kdTree::BoundingBox<Point, R> const &B) const
		{
			//return true;
			double D1 = min_distance(B);
			return D1 < r;
		}
};

template <unsigned R>
class Annulus: public kdTree::Predicate<mVector<double, R>,R>
{
	typedef mVector<double, R> Point;

	Point A;
	double L, a, b;

	public:
		Annulus(double L_, Point const &A_, double a_, double b_): 
			A(A_), L(L_), a(a_*a_), b(b_*b_) {}

		// if for one of the coordinates the reference point is in the same range
		// as the bounding box, that axis does not contribute to the miminum distance
		double min_distance(kdTree::BoundingBox<Point, R> const &B) const
		{
			bool inside = true;
			double d = 0;

			for (unsigned k = 0; k < R; ++k)
			{
				if (A[k] < B.max_coord(k) and A[k] >= B.min_coord(k))
					continue;

				inside = false;
				d += std::min(pow(mod_L(L, A[k] - B.max_coord(k)), 2),
					      pow(mod_L(L, A[k] - B.min_coord(k)), 2));
			}

			return (inside ? -1 : d);
		}

		// the maximum distance is always a corner of the bounding box.
		double max_distance(kdTree::BoundingBox<Point, R> const &B) const
		{
			double d = 0;

			for (unsigned k = 0; k < R; ++k)
			{
				d += std::max(pow(mod_L(L, A[k] - B.max_coord(k)), 2),
					      pow(mod_L(L, A[k] - B.min_coord(k)), 2));
			}

			return d;
		}

		virtual bool operator()(Point const &B) const
		{
			double D = dsqr(L, A, B);
			return a <= D and D < b;
	       	}

		virtual bool operator()(kdTree::BoundingBox<Point, R> const &B) const
		{
			//return true;
			double D1 = min_distance(B), D2 = max_distance(B);
			return not (D1 >= b or D2 < a);
		}
};

template <unsigned R>
class Force: public kdTree::Visitor<mVector<double,R>>
{
	typedef mVector<double, R> Point;

	double L;
	Point A, m_total;

	public:
		Force(double L_, Point const &A_): L(L_), A(A_), m_total(0) {}

		virtual void operator()(Point const &B)
		{
			double D = dsqr(L, A, B);
			if (D > 1e-3) m_total += dist(L, B, A) / pow(D, 3./2);
		}

		Point sum() const { return m_total; }
};

} // namespace Glass

