#pragma once

// CGAL definitions =================================
#include <CGAL/Cartesian.h>
#include <CGAL/Regular_triangulation_euclidean_traits_2.h>
#include <CGAL/Regular_triangulation_2.h>

//#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Regular_triangulation_3.h>
#include <CGAL/Regular_triangulation_euclidean_traits_3.h>

#include "../base/system.hh"
#include <memory>

namespace Adhesion {

template <int rank>
class Adhesion_traits;

template <>
class Adhesion_traits<2>
{
	public:
		typedef CGAL::Cartesian<double> 	K;
		typedef CGAL::Regular_triangulation_euclidean_traits_2<K>  Traits;
		typedef CGAL::Regular_triangulation_2<Traits> RT;

		typedef RT::Point Point;
		typedef RT::Weighted_point Weighted_point;

		typedef typename RT::Segment Segment;
		typedef typename RT::Triangle Triangle;
		typedef typename RT::Vertex Vertex;
		
		static Point dVector2Point(mVector<double, 2> const &p)
			{ return Point(p[0], p[1]); }

		static int face_cnt(
				std::shared_ptr<RT> rt,
				typename RT::Face_handle f)
		{
			Triangle t = rt->triangle(f);
			int c = 0;
			for (unsigned j = 0; j < 3; ++j)
			{
				if (rt->segment(f, j).squared_length() > 3.0)
					++c;
			}
			return c;
		}

		static void _for_each_big_dual_segment(
				std::shared_ptr<RT> rt,
				std::function<void (Segment const &, double)> const &f)
		{
			std::for_each(
				rt->finite_edges_begin(),
				rt->finite_edges_end(),
				[&] (typename RT::Edge const &e)
			{
				CGAL::Object o = rt->dual(e);
				Segment s;
				if (CGAL::assign(s, o))
				{
					f(s, rt->segment(e).squared_length());
				}
			});
		}

		static void _for_each_segment(
				std::shared_ptr<RT> rt,
				std::function<void (Segment const &s)> const &f)
		{
			std::for_each(
				rt->finite_edges_begin(),
				rt->finite_edges_end(),
				[&] (typename RT::Edge const &e)
			{
				f(rt->segment(e));
			});
		}

		static void _for_each_cluster(
				std::shared_ptr<RT> rt,
				std::function<void (Point const &, double)> const &f)
		{
			for(auto i = rt->finite_faces_begin();
				i != rt->finite_faces_end(); ++i)
			{
				Triangle t = rt->triangle(i);
				
				if (face_cnt(rt, i) == 3) 
					f(rt->dual(i), t.area());
			}
		}
};

template <>
class Adhesion_traits<3>
{
	public:
		typedef CGAL::Cartesian<double> 	K;
		typedef CGAL::Regular_triangulation_euclidean_traits_3<K> Traits;
		typedef CGAL::Regular_triangulation_3<Traits> RT;

		typedef RT::Point		Point;
		typedef RT::Weighted_point	Weighted_point;
		typedef RT::Segment		Segment;
		typedef RT::Tetrahedron		Tetrahedron;

		static Point dVector2Point(mVector<double, 3> const &p)
			{ return Point(p[0], p[1], p[2]); }

		static int face_cnt(
				std::shared_ptr<RT> rt,
				RT::Cell_handle const &h)
		{
			int cnt = 0;
			for (unsigned i = 1; i < 4; ++i)
			{
				for (unsigned j = 0; j < i; ++j)
				{
					if (rt->segment(h, i, j).squared_length() / BoxConfig::scale2() > 3.0)
						++cnt;
				}
			}

			return cnt;
		}
				
		static void _for_each_big_dual_segment(
				std::shared_ptr<RT> rt, 
				std::function<void (Segment const &, double)> const &f)
		{
			std::for_each(
				rt->finite_facets_begin(),
				rt->finite_facets_end(),
				[&] (typename RT::Facet const &e)
			{
				CGAL::Object o = rt->dual(e);
				Segment s;
				if (CGAL::assign(s, o))
				{
					if (face_cnt(rt, e.first) + face_cnt(rt, rt->mirror_facet(e).first) >= 10) 
						f(s, rt->triangle(e).squared_area());
				}
			});
		}

		static void _for_each_segment(
				std::shared_ptr<RT> rt,
				std::function<void (Segment const &s)> const &f)
		{
			std::for_each(
				rt->finite_edges_begin(),
				rt->finite_edges_end(),
				[&] (typename RT::Edge const &e)
			{
				f(rt->segment(e));
			});
		}

		static void _for_each_cluster(
				std::shared_ptr<RT> rt,
				std::function<void (Point const &, double)> const &f)
		{
			for(auto i = rt->finite_cells_begin();
				i != rt->finite_cells_end(); ++i)
			{
				if (face_cnt(rt, i) == 6) 
					f(rt->dual(i), rt->tetrahedron(i).volume());
			}
		}
};


template <int R>
class Adhesion: public Adhesion_traits<R>
{
	public:
		typedef typename Adhesion_traits<R>::K 	K;
		typedef typename Adhesion_traits<R>::RT	RT;
		
		typedef typename Adhesion_traits<R>::Weighted_point Weighted_point;
		typedef typename Adhesion_traits<R>::Point		Point;
		
		typedef typename RT::Segment Segment;
		typedef typename RT::Triangle Triangle;
		typedef typename RT::Vertex Vertex;
		
	private:
		std::shared_ptr<RT> rt;
		std::vector<Weighted_point> pts;

		Adhesion():
			rt(new RT)
		{}

	public:
		static std::shared_ptr<Adhesion> create(
			std::shared_ptr<Cube<double>> rf);
		
		template <typename Iter>
		void insert(Iter begin, Iter end)
		{
			std::copy(begin, end, std::back_inserter(pts));
			rt->insert(begin, end);
		}

		void for_each_big_dual_segment(
				std::function<void (Segment const &, double)> const &f)
		{
			Adhesion_traits<R>::_for_each_big_dual_segment(rt, f);
		}

		void for_each_segment(
				std::function<void (Segment const &)> const &f)
		{
			Adhesion_traits<R>::_for_each_segment(rt, f);
		}

		void for_each_cluster(
				std::function<void (Point const &, double)> const &f)
		{
			Adhesion_traits<R>::_for_each_cluster(rt, f);
		}
};

template <int R>
std::shared_ptr<Adhesion<R>> Adhesion<R>::create(
	std::shared_ptr<Cube<double>> rf)
{
	unsigned	N 	= BoxConfig::N();
	float		L	= BoxConfig::L();

	std::vector<Point> X;
	Spaces::GridSpace<double, R> G(N, L);
	std::transform(G.begin(), G.end(), rf->begin(), std::back_inserter(X),
		[] (mVector<double, R> const &x, double w)
	{
		Point Q = Adhesion<R>::dVector2Point(x);
		return Weighted_point(Q, w);
	});

	std::shared_ptr<Adhesion<R>> adh(new Adhesion<R>);
	adh->insert(X.begin(), X.end());

	return adh;
}
}
