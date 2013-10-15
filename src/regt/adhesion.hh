#pragma once

// CGAL definitions =================================
#include <CGAL/Cartesian.h>
#include <CGAL/Regular_triangulation_euclidean_traits_2.h>
#include <CGAL/Regular_triangulation_2.h>

//#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Regular_triangulation_3.h>
#include <CGAL/Regular_triangulation_euclidean_traits_3.h>

#include "../base/system.hh"
#include "../base/boxconfig.hh"
#include "../ply/ply.hh"
#include <memory>

namespace Conan {

using System::mVector;
using System::Array;
using System::ptr;
using System::BoxConfig;
using Misc::PLY;

template <unsigned R>
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

		template <typename F>
		static Point make_Point(F f) { return Point(f(0), f(1)); }

		static int face_cnt(
				System::ptr<System::BoxConfig<2>> box,
				std::shared_ptr<RT> rt,
				typename RT::Face_handle f)
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

		static void _for_each_big_dual_segment(
				System::ptr<System::BoxConfig<2>> box,
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

		static void _for_each_big_dual_face(
				System::ptr<System::BoxConfig<2>> box,
				std::shared_ptr<RT> rt, 
				std::function<void (Array<Point>, double)> const &f)
		{
			std::cerr << "the 2d tessellation has no walls.\n";
			throw "error";
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
				System::ptr<System::BoxConfig<2>> box,
				std::shared_ptr<RT> rt,
				std::function<void (Point const &, double)> const &f)
		{
			for(auto i = rt->finite_faces_begin();
				i != rt->finite_faces_end(); ++i)
			{
				Triangle t = rt->triangle(i);
				
				if (face_cnt(box, rt, i) == 3) 
					f(rt->dual(i), t.area());
			}
		}

		static void _walls_to_ply_file(
			ptr<BoxConfig<2>> box, ptr<RT> rt, 
			std::string const &filename)
		{
			std::cerr << "the 2d tessellation has no walls.\n";
			throw "error";
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

		template <typename F>
		static Point make_Point(F f) { return Point(f(0), f(1), f(2)); }

		static int face_cnt(
				System::ptr<System::BoxConfig<3>> box,
				std::shared_ptr<RT> rt,
				RT::Cell_handle const &h)
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
				
		static void _for_each_big_dual_segment(
				System::ptr<System::BoxConfig<3>> box,
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
					if (face_cnt(box, rt, e.first) + face_cnt(box, rt, rt->mirror_facet(e).first) >= 10) 
						f(s, rt->triangle(e).squared_area());
				}
			});
		}

		static void _for_each_big_dual_face(
				System::ptr<System::BoxConfig<3>> box,
				std::shared_ptr<RT> rt, 
				std::function<void (Array<Point>, double)> const &f)
		{
			std::for_each(
				rt->finite_edges_begin(),
				rt->finite_edges_end(),
				[&] (typename RT::Edge const &e)
			{
				double l = rt->segment(e).squared_length();
				if (l / box->scale2() < 3.0) return;

				auto cells = rt->incident_cells(e), c = cells; ++c;
				if (rt->is_infinite(cells)) return;
				Array<Point> P(0); P->push_back(rt->dual(cells));
				for (; c != cells; ++c)
				{
					if (rt->is_infinite(c)) return;
					else P->push_back(rt->dual(c));
				}

				f(P, l);
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
				System::ptr<System::BoxConfig<3>> box,
				std::shared_ptr<RT> rt,
				std::function<void (Point const &, double)> const &f)
		{
			for(auto i = rt->finite_cells_begin();
				i != rt->finite_cells_end(); ++i)
			{
				if (face_cnt(box, rt, i) == 6) 
					f(rt->dual(i), rt->tetrahedron(i).volume());
			}
		}

		static void _walls_to_ply_file(
			ptr<BoxConfig<3>> box, ptr<RT> rt, 
			std::string const &filename)
		{
			// map Cell_handles to indices into the array of
			// vertices found in the Voronoi diagram.
			std::map<RT::Cell_handle, unsigned> V_map;
			std::vector<Point> V;
			auto cell_dual = [&] (RT::Cell_handle const &h) -> unsigned
			{
				if (V_map.count(h) == 0)
				{
					unsigned q = V.size();
					V_map[h] = q;
					V.push_back(rt->dual(h));
					return q;
				}
				else
				{
					return V_map[h];
				}
			};

			std::vector<std::pair<Array<unsigned>,double>> W;
			std::for_each(
				rt->finite_edges_begin(),
				rt->finite_edges_end(),
				[&] (typename RT::Edge const &e)
			{
				double l = rt->segment(e).squared_length();
				if (l / box->scale2() < 4.0) return;

				Array<unsigned> P(0);
				auto cells = rt->incident_cells(e), c = cells; ++c;
				if (not rt->is_infinite(cells))
					P->push_back(cell_dual(cells));

				for (; c != cells; ++c)
					if (not rt->is_infinite(c))
						P->push_back(cell_dual(c));

				W.push_back(std::pair<Array<unsigned>,double>(P, l));
			});

			PLY ply;
			ply.add_comment("Adhesion model, wall component.");
			
			ply.add_element("vertex", 
				PLY::scalar_type<float>("x"), 
				PLY::scalar_type<float>("y"), 
				PLY::scalar_type<float>("z"));
			for (Point const &v : V)
				ply.put_data(
					PLY::scalar<float>(v[0]),
					PLY::scalar<float>(v[1]),
					PLY::scalar<float>(v[2]));

			ply.add_element("face",
				PLY::list_type<int>("vertex_index"));
				//PLY::scalar_type<float>("density"));
			for (auto f : W)
				if (f.first.size() > 2)
				    ply.put_data(
					PLY::list<int>(f.first));
				//	PLY::scalar<float>(f.second));

			ply.write(filename, PLY::BINARY);
		}
};


template <unsigned R>
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
		System::ptr<System::BoxConfig<R>> box;
		System::ptr<RT> rt;
		std::vector<Weighted_point> pts;

	public:
		Adhesion(System::ptr<System::BoxConfig<R>> box_): 
			box(box_), rt(new RT) {}

		static System::ptr<Adhesion> create(System::ptr<System::BoxConfig<R>>, System::Array<double>, double);
		
		template <typename Iter>
		void insert(Iter begin, Iter end)
		{
			std::copy(begin, end, std::back_inserter(pts));
			rt->insert(pts.begin(), pts.end());
		}

		void for_each_big_dual_segment(
				std::function<void (Segment const &, double)> const &f)
		{
			Adhesion_traits<R>::_for_each_big_dual_segment(box, rt, f);
		}

		void for_each_big_dual_face(
				std::function<void (Array<Point>, double)> const &f)
		{
			Adhesion_traits<R>::_for_each_big_dual_face(box, rt, f);
		}

		void for_each_segment(
				std::function<void (Segment const &)> const &f)
		{
			Adhesion_traits<R>::_for_each_segment(rt, f);
		}

		void for_each_cluster(
				std::function<void (Point const &, double)> const &f)
		{
			Adhesion_traits<R>::_for_each_cluster(box, rt, f);
		}

		void walls_to_ply_file(std::string const &filename)
		{
			Adhesion_traits<R>::_walls_to_ply_file(box, rt, filename);
		}
};

template <unsigned R>
System::ptr<Adhesion<R>> Adhesion<R>::create(
	System::ptr<System::BoxConfig<R>> box, System::Array<double> phi, double t)
{
	System::cVector<R> b(box->bits());

	auto point_set = System::map(System::Range<size_t>(box->size()),
		[&] (size_t x) -> Weighted_point
	{
		Point Q = Adhesion<R>::make_Point(
			[&] (unsigned k) -> double
		{ return box->scale() * b.i(x, k); });

		return Weighted_point(Q, phi[x] * 2 * t);
	});

	/*
	for (auto p : point_set)
	{
		std::cout << p << " " << p.weight() <<  std::endl;
	}
	*/

	auto adh = System::make_ptr<Adhesion<R>>(box);
	adh->insert(point_set.begin(), point_set.end());

	return adh;
}

}
