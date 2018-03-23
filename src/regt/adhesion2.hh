#pragma once

#include <memory>

// CGAL definitions =================================
#include <CGAL/Cartesian.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Regular_triangulation_3.h>
#include <CGAL/Regular_triangulation_2.h>

#include "system.hh"

namespace Conan {

using System::Header;
using System::History;

template <unsigned R>
class Adhesion_base;

template <>
class Adhesion_base<2>
{
	public:
		enum { R = 2 };

		//typedef CGAL::Cartesian<double> 	K;
		typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
		typedef CGAL::Regular_triangulation_2<K> RT;

		typedef RT::Bare_point Point;
		typedef RT::Weighted_point Weighted_point;

		typedef typename RT::Segment Segment;
		typedef typename RT::Triangle Triangle;
		typedef typename RT::Vertex Vertex;
		
		static Point dVector2Point(dVector<2> const &p)
			{ return Point(p[0], p[1]); }

		static dVector<R> Point2dVector(Point const &p)
			{ return dVector<2>({p.x(), p.y()}); }

		template <typename F>
		static Point make_Point(F f) { return Point(f(0), f(1)); }

		typedef RT::Face_handle Node;

	protected:
		BoxPtr<R> box;
		ptr<RT>   rt;

	public:
		Adhesion_base(BoxPtr<R> box_): 
			box(box_), rt(new RT)
		{}

		int face_cnt(RT::Face_handle f)
		{
			Triangle t = rt->triangle(f);
			int c = 0;
			for (unsigned j = 0; j < 3; ++j)
			{
				if (rt->segment(f, j).squared_length() > 3.0 * box->scale2())
					++c;
			}
			return c;
		}

		void for_each_cluster(std::function<void (Point const &, double)> const &f)
		{
			for(auto i = rt->finite_faces_begin();
				i != rt->finite_faces_end(); ++i)
			{
				Triangle t = rt->triangle(i);
				
				if (face_cnt(i) == 3) 
					f(rt->dual(i), t.area());
			}
		}

		double measure(Node n)
		{
			return rt->triangle(n).area();
		}

		Weighted_point point(Node n, int i)
		{
			return n->vertex(i)->point();
		}

		void for_each_node(std::function<void (Node)> f)
		{
			for (auto i  = rt->finite_faces_begin();
				  i != rt->finite_faces_end();
				  ++i) f(i);
		}
};

template <>
class Adhesion_base<3>
{
	public:
		enum { R = 3 };
		//typedef CGAL::Cartesian<double> 	K;
		typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
		typedef CGAL::Regular_triangulation_3<K> RT;

		typedef RT::Bare_point		Point;
		typedef RT::Edge		Edge;
		//typedef RT::Vector		Vector;
		typedef RT::Weighted_point	Weighted_point;
		typedef RT::Segment		Segment;
		typedef RT::Tetrahedron		Tetrahedron;

		static Point dVector2Point(dVector<3> const &p)
			{ return Point(p[0], p[1], p[2]); }

		static dVector<R> Point2dVector(Point const &p)
			{ return dVector<3>({p.x(), p.y(), p.z()}); }

		template <typename F>
		static Point make_Point(F f) { return Point(f(0), f(1), f(2)); }

		typedef RT::Cell_handle		Node;

	protected:
		System::ptr<System::Box<R>> box;
		System::ptr<RT> rt;

	public:
		Adhesion_base(BoxPtr<R> box_): 
			box(box_), rt(new RT)
		{}

		int face_cnt(RT::Cell_handle h)
		{
			int cnt = 0;
			for (unsigned i = 1; i < 4; ++i)
			{
				for (unsigned j = 0; j < i; ++j)
				{
					if (rt->segment(h, i, j).squared_length() / box->scale2() > 3.0)
						++cnt;
				}
			}

			return cnt;
		}

		double measure(Node h)
		{
			return rt->tetrahedron(h).volume();
		}

		Weighted_point point(Node n, int i)
		{
			return rt->point(n, i);
		}

		void for_each_node(std::function<void (Node)> f)
		{
			for (auto i  = rt->finite_cells_begin();
				  i != rt->finite_cells_end();
				  ++i) f(i);
		}
};


template <unsigned R_>
class Adhesion: public Adhesion_base<R_>
{
	public:
		using Base = Adhesion_base<R_>;

		using Base::R;
		using RT    		= typename Base::RT;
		using Point		= typename Base::Point;
		using Weighted_point	= typename Base::Weighted_point;
		using Node              = typename Base::Node;

		using Base::rt;
		using Base::box;
		
	protected:
		std::vector<Weighted_point> pts;

	public:
		Adhesion(BoxPtr<R> box_):
			Base(box_) {}

		void from_potential(Array<double> phi, double t)
		{
			auto point_set = System::map(System::Range<size_t>(box->size()),
				[&] (size_t x) -> Weighted_point
			{
				System::mVector<double,R> p = box->G[x];
				Point Q = Base::make_Point(
					[&] (unsigned k) -> double { return p[k]; });
				return Weighted_point(Q, phi[x] * 2 * t);
			});

			insert(point_set.begin(), point_set.end());
		}

		void from_potential_with_glass(Array<dVector<R>> glass,
			Array<double> phi, double t)
		{
			Misc::Interpol::Linear<Array<double>,R> pot(box, phi);

			auto point_set = System::map(glass,
				[&] (System::mVector<double,R> const &x) -> Weighted_point
			{
				Point Q = Base::make_Point(
					[&] (unsigned k) -> double { return x[k]; });

				return Weighted_point(Q, pot(x / box->scale()) * 2 * t);
			});

			insert(point_set.begin(), point_set.end());
		}

		template <typename Iter>
		void insert(Iter begin, Iter end)
		{
			std::copy(begin, end, std::back_inserter(pts));
			rt->insert(pts.begin(), pts.end());
		}

		virtual void save_all(Header const &H)
		{}
};

}
