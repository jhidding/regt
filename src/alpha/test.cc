#include "../base/system.hh"
#include "../base/format.hh"

// CGAL definitions =================================
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Regular_triangulation_euclidean_traits_2.h>
#include <CGAL/Regular_triangulation_2.h>
#include <CGAL/Regular_triangulation_euclidean_traits_3.h>
#include <CGAL/Regular_triangulation_3.h>

#include "Alpha_shape_3.h"
#include <vector>
#include <utility>
#include <initializer_list>
#include <algorithm>
#include <iterator>

template <typename T, unsigned N>
class OMultiple: public std::vector<T>
{
	public:
		OMultiple(std::initializer_list<T> const &X):
			std::vector<T>(X) 
		{
			std::sort(this->begin(), this->end());
		}

		bool operator<(OMultiple const &o) const
		{
			for (unsigned i = 0; i < N; ++i)
				if ((*this)[i] < o[i]) return true;
				else if ((*this)[i] > o[i]) return false;

			return false;
		}

		bool operator==(OMultiple const &o) const
		{
			for (unsigned i = 0; i < N; ++i)
				if ((*this)[i] != o[i]) return false;
			return false;
		}
};

template <typename T, unsigned N>
std::ostream &operator<<(std::ostream &out, OMultiple<T,N> const &A)
{
	//out << N;
	for (auto a : A) out << " " << a;

	return out;
}

template <unsigned R>
class Alpha;

template <unsigned R>
class Weighted_Alpha;

template <>
class Weighted_Alpha<3>
{
	public:
		typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
		//typedef CGAL::Cartesian<double> 	K;
		typedef CGAL::Regular_triangulation_euclidean_traits_3<K> Gt;
		typedef CGAL::Alpha_shape_vertex_base_3<Gt> Vb;

		
		typedef CGAL::Alpha_shape_cell_base_3<Gt> Fb;
		typedef CGAL::Triangulation_data_structure_3<Vb,Fb> Tds;
		typedef CGAL::Regular_triangulation_3<Gt,Tds> RT;
		typedef CGAL::Delaunay_triangulation_3<K,Tds,CGAL::Fast_location> Delaunay;
		typedef CGAL::Alpha_shape_3<RT> Alpha_shape;

		typedef Alpha_shape::Cell_handle Cell_handle;
		typedef Alpha_shape::Vertex_handle Vertex_handle;
		typedef Alpha_shape::Facet Facet;
		typedef Alpha_shape::Edge Edge;
		typedef K::Point_3 Point;

		typedef Gt::Weighted_point Weighted_point;
		typedef Gt::Bare_point Bare_point;

		template <typename Pts>
		static System::ptr<Alpha_shape> from_weighted_points(Pts S)
		{
			return System::make_ptr<Alpha_shape>(S.begin(), S.end(), 0, Alpha_shape::GENERAL);
		}

		class Phat_writer: public std::iterator<std::output_iterator_tag,CGAL::Object>
		{
			std::ostream &fo_bmatrix, &fo_points;

			// Vertex_handles are sortable. However edges and facets
			// have multiple valid representations. They are stored as
			// combinations of Cell_handle and vertex indices. To be able
			// to identify them we need to be able to search on the constituent
			// vertices, stored in an ordered multiple.
			System::ptr<std::map<Vertex_handle,size_t>> vm;	// vertices
			System::ptr<std::map<OMultiple<size_t,2>,size_t>>  em;	// edges
			System::ptr<std::map<OMultiple<size_t,3>,size_t>>  fm;	// facets
			System::ptr<size_t> idx;

			public:
				Phat_writer(std::ostream &fo_bmatrix_, std::ostream &fo_points_):
					fo_bmatrix(fo_bmatrix_), 
					fo_points(fo_points_), 
					vm(new std::map<Vertex_handle,size_t>),
					em(new std::map<OMultiple<size_t,2>, size_t>),
					fm(new std::map<OMultiple<size_t,3>, size_t>),
					idx(new size_t(0))
				{}

				Phat_writer &operator()(double alpha, Vertex_handle v)
				{
					(*vm)[v] = *idx;
					fo_bmatrix << "0\n";
					fo_points << alpha << " 0 " << v->point() << std::endl;
					++(*idx);
					return *this;
				}

				Phat_writer &operator()(double alpha, Edge e)
				{
					Vertex_handle v1 = e.first->vertex(e.second),
						      v2 = e.first->vertex(e.third);
					OMultiple<size_t,2> edge({(*vm)[v1], (*vm)[v2]});
					(*em)[edge] = *idx;
					fo_bmatrix << "2 " << edge << std::endl;
					fo_points << alpha << " 1 " << edge << std::endl;
					++(*idx);
					return *this;
				}

				Phat_writer &operator()(double alpha, Facet f)
				{
					std::vector<size_t> v;
					for (unsigned i = 0; i < 4; ++i)
						if (i != f.second) v.push_back((*vm)[f.first->vertex(i)]);
					OMultiple<size_t,2> e1({v[0], v[1]}), 
							    e2({v[0], v[2]}), 
							    e3({v[1], v[2]});
					OMultiple<size_t,3> facet({v[0],v[1],v[2]});

					fo_bmatrix << "3 " << (*em)[e1] << " " << (*em)[e2] << " " 
						   << (*em)[e3] << std::endl;
					(*fm)[facet] = *idx;
					fo_points << alpha << " 2 " << facet << std::endl;
					++(*idx);
					return *this;
				}

				Phat_writer &operator()(double alpha, Cell_handle c)
				{
					OMultiple<size_t, 4> v({
						(*vm)[c->vertex(0)],
						(*vm)[c->vertex(1)],
						(*vm)[c->vertex(2)],
						(*vm)[c->vertex(3)]});

					OMultiple<size_t,3> f1({v[0], v[1], v[2]}),
							    f2({v[0], v[1], v[3]}),
							    f3({v[0], v[2], v[3]}),
							    f4({v[1], v[2], v[3]});

					fo_bmatrix << "4 " << (*fm)[f1] << " " << (*fm)[f2] << " " 
						   << (*fm)[f3] << " " << (*fm)[f4] << std::endl;

					fo_points << alpha << " 3 " << v << std::endl;
					++(*idx);
					return *this;
				}
		};

		static void write_persistence(
			System::ptr<Alpha_shape> as,
			std::string const &fn_bmatrix,
			std::string const &fn_points)
		{
			std::ofstream fo_bmatrix(fn_bmatrix), fo_points(fn_points);
			as->get_filtration(Phat_writer(fo_bmatrix, fo_points));
			fo_bmatrix.close(); fo_points.close();
		}
};

template <>
class Alpha<3>
{
	public:
		//typedef CGAL::Cartesian<double> 	K;
		typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
		typedef CGAL::Exact_predicates_inexact_constructions_kernel Gt;
		//typedef CGAL::Regular_triangulation_euclidean_traits_3<K> Gt;
		typedef CGAL::Alpha_shape_vertex_base_3<Gt> Vb;

		
		typedef CGAL::Alpha_shape_cell_base_3<Gt> Fb;
		typedef CGAL::Triangulation_data_structure_3<Vb,Fb> Tds;
		//typedef CGAL::Regular_triangulation_3<Gt,Tds> RT;
		typedef CGAL::Delaunay_triangulation_3<K,Tds,CGAL::Fast_location> Delaunay;
		typedef CGAL::Alpha_shape_3<Delaunay> Alpha_shape;

		typedef Delaunay::Cell_handle Cell_handle;
		typedef Delaunay::Vertex_handle Vertex_handle;
		typedef Delaunay::Facet Facet;
		typedef Delaunay::Edge Edge;
		typedef K::Point_3 Point;

		//typedef Gt::Weighted_point Weighted_point;
		//typedef Gt::Bare_point Bare_point;

		template <typename Pts>
		static System::ptr<Alpha_shape> from_weighted_points(Pts S)
		{
			return System::make_ptr<Alpha_shape>(S.begin(), S.end(), 0, Alpha_shape::GENERAL);
		}

		class Phat_writer: public std::iterator<std::output_iterator_tag,CGAL::Object>
		{
			std::ostream &fo_bmatrix, &fo_points;

			// Vertex_handles are sortable. However edges and facets
			// have multiple valid representations. They are stored as
			// combinations of Cell_handle and vertex indices. To be able
			// to identify them we need to be able to search on the constituent
			// vertices, stored in an ordered multiple.
			System::ptr<std::map<Vertex_handle,size_t>> vm;	// vertices
			System::ptr<std::map<OMultiple<size_t,2>,size_t>>  em;	// edges
			System::ptr<std::map<OMultiple<size_t,3>,size_t>>  fm;	// facets
			System::ptr<size_t> idx;

			public:
				Phat_writer(std::ostream &fo_bmatrix_, std::ostream &fo_points_):
					fo_bmatrix(fo_bmatrix_), 
					fo_points(fo_points_), 
					vm(new std::map<Vertex_handle,size_t>),
					em(new std::map<OMultiple<size_t,2>, size_t>),
					fm(new std::map<OMultiple<size_t,3>, size_t>),
					idx(new size_t(0))
				{}

				Phat_writer &operator()(double alpha, Vertex_handle v)
				{
					(*vm)[v] = *idx;
					fo_bmatrix << "0\n";
					fo_points << alpha << " 0 " << v->point() << std::endl;
					++(*idx);
					return *this;
				}

				Phat_writer &operator()(double alpha, Edge e)
				{
					Vertex_handle v1 = e.first->vertex(e.second),
						      v2 = e.first->vertex(e.third);
					OMultiple<size_t,2> edge({(*vm)[v1], (*vm)[v2]});
					(*em)[edge] = *idx;
					fo_bmatrix << "2 " << edge << std::endl;
					fo_points << alpha << " 1 " << edge << std::endl;
					++(*idx);
					return *this;
				}

				Phat_writer &operator()(double alpha, Facet f)
				{
					std::vector<size_t> v;
					for (unsigned i = 0; i < 4; ++i)
						if (i != f.second) v.push_back((*vm)[f.first->vertex(i)]);
					OMultiple<size_t,2> e1({v[0], v[1]}), 
							    e2({v[0], v[2]}), 
							    e3({v[1], v[2]});
					OMultiple<size_t,3> facet({v[0],v[1],v[2]});

					fo_bmatrix << "3 " << (*em)[e1] << " " << (*em)[e2] << " " 
						   << (*em)[e3] << std::endl;
					(*fm)[facet] = *idx;
					fo_points << alpha << " 2 " << facet << std::endl;
					++(*idx);
					return *this;
				}

				Phat_writer &operator()(double alpha, Cell_handle c)
				{
					OMultiple<size_t, 4> v({
						(*vm)[c->vertex(0)],
						(*vm)[c->vertex(1)],
						(*vm)[c->vertex(2)],
						(*vm)[c->vertex(3)]});

					OMultiple<size_t,3> f1({v[0], v[1], v[2]}),
							    f2({v[0], v[1], v[3]}),
							    f3({v[0], v[2], v[3]}),
							    f4({v[1], v[2], v[3]});

					fo_bmatrix << "4 " << (*fm)[f1] << " " << (*fm)[f2] << " " 
						   << (*fm)[f3] << " " << (*fm)[f4] << std::endl;

					fo_points << alpha << " 3 " << v << std::endl;
					++(*idx);
					return *this;
				}
		};

		static void write_persistence(
			System::ptr<Alpha_shape> as,
			std::string const &fn_bmatrix,
			std::string const &fn_points)
		{
			std::ofstream fo_bmatrix(fn_bmatrix), fo_points(fn_points);
			as->get_filtration(Phat_writer(fo_bmatrix, fo_points));
			fo_bmatrix.close(); fo_points.close();
		}
};

std::ostream &operator<<(std::ostream &out, CGAL::Object const &obj)
{
	Alpha<3>::Vertex_handle v;
	if (CGAL::assign(v, obj))
	{
		out << v->point();
	}
	return out;
}


using namespace System;

void cmd_alpha(int argc, char **argv)
{
	std::ostringstream ss;
	ss << time(NULL);
	std::string timed_seed = ss.str();

	Argv C = read_arguments(argc, argv,
		Option({0, "h", "help", "false",
			"print help on the use of this program."}),

		/*Option({Option::VALUED | Option::CHECK, "d", "dim", "2",
			"number of dimensions in which to run the code. "
			"Usually this should by 2 or 3."}),*/

		Option({0, "w", "weighted", "false",
			"use the masses in the table as weights."}),
		Option({Option::VALUED | Option::CHECK, "i", "id", date_string(),
			"identifier for filenames."}),
	
		Option({Option::VALUED | Option::CHECK, "t", "time", "1.0",
			"growing mode parameter."}));

	if (C.get<bool>("help"))
	{
		std::cout << "Cosmic workset Conan, by Johan Hidding.\n\n";
		C.print(std::cout);
		exit(0);
	}

	double time = C.get<double>("time");
	std::string fn_input = timed_filename(C["id"], "regt", time),
		    fn_bmatrix = timed_filename(C["id"], "bmatrix", time),
		    fn_alpha = timed_filename(C["id"], "alpha", time);

	// read initial potential from file.
	std::ifstream fi;
	std::cerr << "reading " << fn_input << " ... ";
	fi.open(fn_input.c_str(), std::ios::in);
	std::string line;

	if (C.get<bool>("weighted"))
	{
		Array<Weighted_Alpha<3>::Weighted_point> pts(0);
		while (not std::getline(fi, line).fail())
		{
			Weighted_Alpha<3>::Weighted_point wp;
			if (line == "" or line[0] == '#') continue;
			std::istringstream ss(line); ss >> wp;
			pts->push_back(Weighted_Alpha<3>::Weighted_point(wp.point(), pow(wp.weight(), 2./3)/10.));
		}
		std::cerr << pts.size() << " points read.\n";
		std::cerr << *pts.begin() << " ... " << pts->back() << std::endl;
		fi.close();

		std::cerr << "Computing alpha-shape ... ";
		auto as = Weighted_Alpha<3>::from_weighted_points(pts);
		std::cerr << "[done]\n";
		Weighted_Alpha<3>::write_persistence(as, fn_bmatrix, fn_alpha);
	}
	else
	{
		Array<Alpha<3>::Point> pts(0);
		while (not std::getline(fi, line).fail())
		{
			Alpha<3>::Point wp;
			if (line == "" or line[0] == '#') continue;
			std::istringstream ss(line); ss >> wp;
			pts->push_back(wp);
		}
		std::cerr << pts.size() << " points read.\n";
		std::cerr << *pts.begin() << " ... " << pts->back() << std::endl;
		fi.close();

		std::cerr << "Computing alpha-shape ... ";
		auto as = Alpha<3>::from_weighted_points(pts);
		std::cerr << "[done]\n";
		Alpha<3>::write_persistence(as, fn_bmatrix, fn_alpha);
	}
}

Global<Command> _ALPHA("alpha", cmd_alpha);

