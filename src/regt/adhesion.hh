#pragma once

// CGAL definitions =================================
#include <CGAL/Cartesian.h>
#include <CGAL/Regular_triangulation_euclidean_traits_2.h>
#include <CGAL/Regular_triangulation_2.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Regular_triangulation_3.h>
#include <CGAL/Regular_triangulation_euclidean_traits_3.h>

#include "../base/system.hh"
#include "../base/box.hh"
#include "../ply/ply.hh"
#include "../misc/interpol.hh"
#include <memory>

namespace Conan {

using System::mVector;
using System::Array;
using System::ptr;
using System::Box;
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
				System::ptr<System::Box<2>> box,
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
				System::ptr<System::Box<2>> box,
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
				System::ptr<System::Box<2>> box,
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
				System::ptr<System::Box<2>> box,
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

		static void _filam_to_ply_file(
			ptr<Box<2>> box, ptr<RT> rt, 
			std::string const &filename)
		{
			std::cerr << "the 2d tessellation has no meaningfull 3d representation.\n";
			throw "error";
		}

		static void _walls_to_ply_file(
			ptr<Box<2>> box, ptr<RT> rt, 
			std::string const &filename)
		{
			std::cerr << "the 2d tessellation has no walls.\n";
			throw "error";
		}

		static void _write_persistence(
			ptr<Box<2>> box, ptr<RT> rt,
			std::string const &fn_bmatrix, std::string const &fn_points,
			std::string const &fn_values)
		{
			std::cerr << "not yet implemented.\n";
			throw "error";
		}
};

template <typename T>
class OPair: public std::pair<T,T>
{
	public:
		OPair(T a, T b)
		{
			if (a < b) { this->first = a; this->second = b; }
			else { this->first = b; this->second = a; }
		}

		bool operator<(OPair const &o) const
		{
			if (this->first < o.first) return true;
			if (this->first == o.first) return this->second < o.second;
			return false;
		}

		bool operator==(OPair const &o) const
		{
			return this->first == o.first and this->second == o.second;
		}
};

template <typename T>
std::ostream &operator<<(std::ostream &out, OPair<T> const &p)
{
	return out << p.first << " " << p.second;
}

template <>
class Adhesion_traits<3>
{
	public:
		//typedef CGAL::Cartesian<double> 	K;
		typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
		typedef CGAL::Regular_triangulation_euclidean_traits_3<K> Traits;
		typedef CGAL::Regular_triangulation_3<Traits> RT;

		typedef RT::Point		Point;
		typedef RT::Edge		Edge;
		//typedef RT::Vector		Vector;
		typedef RT::Weighted_point	Weighted_point;
		typedef RT::Segment		Segment;
		typedef RT::Tetrahedron		Tetrahedron;

		static Point dVector2Point(mVector<double, 3> const &p)
			{ return Point(p[0], p[1], p[2]); }

		template <typename F>
		static Point make_Point(F f) { return Point(f(0), f(1), f(2)); }

		static int face_cnt(
				System::ptr<System::Box<3>> box,
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
				System::ptr<System::Box<3>> box,
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
				System::ptr<System::Box<3>> box,
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
				System::ptr<System::Box<3>> box,
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

		static void _write_persistence(
			ptr<Box<3>> box, ptr<RT> rt,
			std::string const &fn_bmatrix, std::string const &fn_points,
			std::string const &fn_values)
		{
			std::map<RT::Cell_handle, unsigned> V_map;
			std::vector<Point> V; std::vector<double> M;

			auto cell_dual = [&] (RT::Cell_handle const &h) -> unsigned
			{
				if (V_map.count(h) == 0)
				{
					unsigned q = V.size();
					V_map[h] = q;
					V.push_back(rt->dual(h));
					M.push_back(rt->tetrahedron(h).volume());
					return q;
				}
				else
				{
					return V_map[h];
				}
			};

			auto is_cell_ok = [&] (RT::Cell_handle const &h) -> bool
			{
				if (rt->is_infinite(h)) return false;
				for (unsigned k = 0; k < 4; ++k)
				{
					Point p = rt->point(h, k);
					for (unsigned k = 0; k < 3; ++k)
						if ((p[k] > 0.9 * box->L()) or (p[k] < 0.1 * box->L()))
							return false;
				}

				return true;
			};

			std::vector<std::pair<RT::Cell_handle,double>> cells;
			for (auto fc = rt->finite_cells_begin(); fc != rt->finite_cells_end(); ++fc)
			{
				if (is_cell_ok(fc))
					cells.push_back(std::pair<RT::Cell_handle,double>(fc, 
						rt->tetrahedron(fc).volume()));
			}
			/*
			auto cell_adder = [&] (RT::Cell_handle const &h)
			{
				if (is_cell_ok(h))
					cells.push_back(std::pair<RT::Cell_handle,double>(h, rt->tetrahedron(h).volume()));
			};
			std::for_each(rt->finite_cells_begin(), rt->finite_cells_end(), cell_adder);
			*/
			std::sort(cells.begin(), cells.end(),
				[&] (std::pair<RT::Cell_handle,double> const &a, 
				     std::pair<RT::Cell_handle,double> const &b)
			{
				return b.second < a.second; // sort descending;
			});

			std::vector<std::pair<RT::Facet, double>> facets;
			std::for_each(rt->finite_facets_begin(), rt->finite_facets_end(),
				[&] (RT::Facet const &f)
			{
				RT::Cell_handle h1 = f.first, h2 = rt->mirror_facet(f).first;
				if (is_cell_ok(h1) and is_cell_ok(h2))
					facets.push_back(std::pair<RT::Facet,double>(f,
						std::min(rt->tetrahedron(h1).volume(),
							rt->tetrahedron(h2).volume())));
			});
			std::sort(facets.begin(), facets.end(),
				[&] (std::pair<RT::Facet,double> const &a, 
				     std::pair<RT::Facet,double> const &b)
			{
				return b.second < a.second; // sort descending;
			});

			std::vector<std::pair<RT::Edge, double>> edges;
			std::for_each(rt->finite_edges_begin(), rt->finite_edges_end(),
				[&] (RT::Edge const &e)
			{
				auto cells = rt->incident_cells(e), c = cells; ++c;
				double v; 

				if (is_cell_ok(cells))
					v = rt->tetrahedron(cells).volume();
				else return;

				for (; c != cells; ++c)
					if (is_cell_ok(c))
						v = std::min(v, rt->tetrahedron(c).volume());
					else return;

				edges.push_back(std::pair<RT::Edge, double>(e, v));
			});
			std::sort(edges.begin(), edges.end(),
				[&] (std::pair<RT::Edge,double> const &a, 
				     std::pair<RT::Edge,double> const &b)
			{
				return b.second < a.second; // sort descending;
			});

			size_t ei = 0, fi = 0, ci = 0, oc = 0;
			std::map<RT::Cell_handle, size_t> cm;
			std::map<OPair<size_t>, size_t> fm;
			auto get_facet_idx = [&] (RT::Facet const &f)
			{
				auto c1 = f.first, c2 = rt->mirror_facet(f).first;
				return fm[OPair<size_t>(cm[c1], cm[c2])];
			};

			std::ofstream fo_bmatrix(fn_bmatrix), fo_points(fn_points), fo_values(fn_values);
			while (ei < edges.size())
			{
				if (ci < cells.size() and cells[ci].second >= facets[fi].second and cells[ci].second >= edges[ei].second)
				{
					// write cell, vertex in E
					cm[cells[ci].first] = oc;
					fo_bmatrix << "0\n";
					fo_points << rt->dual(cells[ci].first) << " " 
						<< cells[ci].second << " " 
						<< oc << std::endl;
					fo_values << cells[ci].second << std::endl;
					++ci; ++oc;
					continue;
				}

				if (fi < facets.size() and facets[fi].second >= edges[ei].second)
				{
					auto c1 = facets[fi].first.first,
					     c2 = rt->mirror_facet(facets[fi].first).first;
					// make OTuple from bounding cells of the facet
					OPair<size_t> c(cm[c1], cm[c2]);
					fm[c] = oc;

					fo_bmatrix << "2 " << c << std::endl;
					fo_values << facets[fi].second << std::endl;
					++fi; ++oc;
					continue;
				}

				// else we add an edge
				std::vector<size_t> C;
				auto ifacs = rt->incident_facets(edges[ei].first), f = ifacs; ++f;
				C.push_back(get_facet_idx(*ifacs));
				for (; f != ifacs; ++f)
					C.push_back(get_facet_idx(*f));

				fo_bmatrix << C.size();
				for (size_t i : C) fo_bmatrix << " " << i;
				fo_bmatrix << std::endl;
				fo_values << edges[ei].second << std::endl;
				++ei; ++oc;
			}

			fo_bmatrix.close(); fo_points.close(); fo_values.close();
		}

		static void _filam_to_ply_file(
			ptr<Box<3>> box, ptr<RT> rt,
			std::string const &filename)
		{
			// map Cell_handles to indices into the array of
			// vertices found in the Voronoi diagram.
			std::map<RT::Cell_handle, unsigned> V_map;
			std::vector<Point> V; std::vector<double> M;
			auto cell_dual = [&] (RT::Cell_handle const &h) -> unsigned
			{
				if (V_map.count(h) == 0)
				{
					unsigned q = V.size();
					V_map[h] = q;
					V.push_back(rt->dual(h));
					M.push_back(rt->tetrahedron(h).volume());
					return q;
				}
				else
				{
					return V_map[h];
				}
			};

			auto is_cell_ok = [&] (RT::Cell_handle const &h) -> bool
			{
				if (rt->is_infinite(h)) return false;
				for (unsigned k = 0; k < 4; ++k)
				{
					Point p = rt->point(h, k);
					for (unsigned k = 0; k < 3; ++k)
						if ((p[k] > box->L()) or (p[k] < 0))
							return false;
				}

				return true;
			};

			std::vector<std::pair<Array<unsigned>,double>> W;
			std::for_each(
				rt->finite_facets_begin(),
				rt->finite_facets_end(),
				[&] (typename RT::Facet const &f)
			{
				double l = rt->triangle(f).squared_area();
				if (l / (box->scale2()*box->scale2()) < 6.0) return;

				Array<unsigned> P(0);
				auto c1 = f.first, c2 = rt->mirror_facet(f).first;
				if (not (is_cell_ok(c1) and is_cell_ok(c2))) return;
				if (face_cnt(box, rt, c1) < 5 and face_cnt(box, rt, c2) < 5) return;

				P->push_back(cell_dual(c1));
				P->push_back(cell_dual(c2));

				W.push_back(std::pair<Array<unsigned>,double>(P, l));
			});

			PLY ply;
			ply.add_comment("Adhesion model, filament component.");
			
			ply.add_element("vertex", 
				PLY::scalar_type<float>("x"), 
				PLY::scalar_type<float>("y"), 
				PLY::scalar_type<float>("z"));
			for (Point const &v : V)
				ply.put_data(
					PLY::scalar<float>(v[0]),
					PLY::scalar<float>(v[1]),
					PLY::scalar<float>(v[2]));

			ply.add_element("edge",
				PLY::scalar_type<int>("vertex1"),
				PLY::scalar_type<int>("vertex2"),
				PLY::scalar_type<float>("density"));
			for (auto f : W)
				ply.put_data(
					PLY::scalar<int>(f.first[0]),
					PLY::scalar<int>(f.first[1]),
					PLY::scalar<float>(f.second));

			ply.write(filename, PLY::BINARY);
		}

		static void _walls_to_ply_file(
			ptr<Box<3>> box, ptr<RT> rt, 
			std::string const &filename)
		{
			// map Cell_handles to indices into the array of
			// vertices found in the Voronoi diagram.
			std::map<RT::Cell_handle, unsigned> V_map;
			std::vector<Point> V; std::vector<double> M;
			auto cell_dual = [&] (RT::Cell_handle const &h) -> unsigned
			{
				if (V_map.count(h) == 0)
				{
					unsigned q = V.size();
					V_map[h] = q;
					V.push_back(rt->dual(h));
					M.push_back(rt->tetrahedron(h).volume());
					return q;
				}
				else
				{
					return V_map[h];
				}
			};

			auto is_cell_ok = [&] (RT::Cell_handle const &h) -> bool
			{
				if (rt->is_infinite(h)) return false;
				for (unsigned k = 0; k < 4; ++k)
				{
					Point p = rt->point(h, k);
					for (unsigned k = 0; k < 3; ++k)
						if ((p[k] > box->L()) or (p[k] < 0))
							return false;
				}

				return true;
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
				if (is_cell_ok(cells))
					P->push_back(cell_dual(cells));

				for (; c != cells; ++c)
					if (is_cell_ok(c))
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
				PLY::list_type<int>("vertex_index"),
				PLY::scalar_type<float>("density"));
			for (auto f : W)
				if (f.first.size() > 2)
				    ply.put_data(
					PLY::list<int>(f.first),
					PLY::scalar<float>(f.second));

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
		System::ptr<System::Box<R>> box;
		System::ptr<RT> rt;
		std::vector<Weighted_point> pts;

	public:
		Adhesion(System::ptr<System::Box<R>> box_): 
			box(box_), rt(new RT) {}

		static System::ptr<Adhesion> create(System::ptr<System::Box<R>>, System::Array<double>, double);
		static System::ptr<Adhesion<R>> create_from_glass(
				System::ptr<System::Box<R>> box, 
				System::Array<System::mVector<double,R>> glass, 
				System::Array<double> phi, double t);
		
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

		void filam_to_ply_file(std::string const &filename)
		{
			Adhesion_traits<R>::_filam_to_ply_file(box, rt, filename);
		}

		void walls_to_ply_file(std::string const &filename)
		{
			Adhesion_traits<R>::_walls_to_ply_file(box, rt, filename);
		}

		void write_persistence(
			std::string const &fn_bmatrix, std::string const &fn_points, 
			std::string const &fn_values)
		{
			Adhesion_traits<R>::_write_persistence(box, rt, fn_bmatrix, fn_points, fn_values);
		}
};

template <unsigned R>
System::ptr<Adhesion<R>> Adhesion<R>::create(
	System::ptr<System::Box<R>> box, System::Array<double> phi, double t)
{
	auto point_set = System::map(System::Range<size_t>(box->size()),
		[&] (size_t x) -> Weighted_point
	{
		System::mVector<double,R> p = box->G[x];
		Point Q = Adhesion<R>::make_Point(
			[&] (unsigned k) -> double { return p[k]; });
		return Weighted_point(Q, phi[x] * 2 * t);
	});

	auto adh = System::make_ptr<Adhesion<R>>(box);
	adh->insert(point_set.begin(), point_set.end());

	return adh;
}

template <unsigned R>
System::ptr<Adhesion<R>> Adhesion<R>::create_from_glass(
	System::ptr<System::Box<R>> box, 
	System::Array<System::mVector<double,R>> glass, 
	System::Array<double> phi, double t)
{
	Misc::Interpol::Linear<Array<double>,R> pot(box, phi);

	auto point_set = System::map(glass,
		[&] (System::mVector<double,R> const &x) -> Weighted_point
	{
		Point Q = Adhesion<R>::make_Point(
			[&] (unsigned k) -> double { return x[k]; });

		return Weighted_point(Q, pot(x / box->scale()) * 2 * t);
	});

	auto adh = System::make_ptr<Adhesion<R>>(box);
	adh->insert(point_set.begin(), point_set.end());

	return adh;
}

}
