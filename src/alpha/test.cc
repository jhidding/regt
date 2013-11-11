
// CGAL definitions =================================
#include <CGAL/Cartesian.h>
#include <CGAL/Regular_triangulation_euclidean_traits_2.h>
#include <CGAL/Regular_triangulation_2.h>
#include <CGAL/Regular_triangulation_euclidean_traits_3.h>
#include <CGAL/Regular_triangulation_3.h>

#include <CGAL/Alpha_shape_3.h>
#include <vector>
#include <utility>
#include <initializer_list>
#include <algorithm>

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

template <typename T, int N>
class OMultiple: public std::vector<T>
{
	public:
		OMultiple(std::initializer_list<T> const &X):
			std::vector<T>(X) 
		{
			std::sort(begin(), end());
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


template <unsigned R>
class Alpha;

template <>
class Alpha<3>
{
	public:
		typedef CGAL::Cartesian<double> 	K;
		typedef CGAL::Regular_triangulation_euclidean_traits_3<K> Gt;
		typedef CGAL::Alpha_shape_vertex_base_3<Gt> Vb;

		typedef CGAL::Alpha_shape_cell_base_3<Gt> Fb;
		typedef CGAL::Triangulation_data_structure_3<Vb,Fb> Tds;
		typedef CGAL::Regular_triangulation_3<Gt,Tds> RT;
		typedef CGAL::Alpha_shape_3<RT> Alpha_shape;

		typedef Alpha_shape::Cell_handle Cell_handle;
		typedef Alpha_shape::Vertex_handle Vertex_handle;
		typedef Alpha_shape::Facet Facet;
		typedef Alpha_shape::Edge Edge;
		typedef Gt::Weighted_point Weighted_point;
		typedef Gt::Bare_point Bare_point;

		template <typename Pts>
		static ptr<Alpha_shape> from_weighted_points(Pts S)
		{
			return make_ptr<Alpha_shape>(S.begin(), S.end(), 0, Alpha_shape_3::GENERAL);
		}

		class Phat_writer: public std::iterator<std::output_iterator_tag,CGAL::Object>
		{
			std::ostream &fo_bmatrix, &fo_points;

			// Vertex_handles are sortable. However edges and facets
			// have multiple valid representations. They are stored as
			// combinations of Cell_handle and vertex indices. To be able
			// to identify them we need to be able to search on the constituent
			// vertices, stored in an ordered multiple.
			std::map<Vertex_handle,size_t> vm;	// vertices
			std::map<OMultiple<size_t,2>>  em;	// edges
			std::map<OMultiple<size_t,3>>  fm;	// facets
			size_t idx;

			CGAL::Object input;

			public:
				Phat_writer(std::ostream &fo_bmatrix_, std::ostream &fo_points_):
					fo_bmatrix(fo_bmatrix_), fo_points(fo_points_)
				{}

				CGAL::Object &operator*() { return input; }

				Phat_writer operator++()
				{
					Vertex_handle v;
					if (CGAL::assign(v, input))
					{
						vm[v] = idx;
						fo_bmatrix << "0\n";
						++idx;
						return *this;
					}

					Edge e;
					if (CGAL::assign(e, input))
					{
						Vertex_handle v1 = e.first->vertex(e.second),
						              v2 = e.first->vertex(e.third);
						OMultiple<size_t,2> edge({vm[v1], vm[v2]});
						em[edge] = idx;
						fo_bmatrix << edge << std::endl;
						++idx;
						return *this;
					}

					Facet f;
					if (CGAL::assign(f, input))
					{
						// this is wrong, i need the EDGES
						std::vector<Vertex_handle> v;
						for (unsigned i = 0; i < 4; ++i)
							if (i != f.second) v.push_back(f.first->vertex(i));
						OMultiple<size_t,3> facet({vm[v[0]], vm[v[1]], vm[v[2]]});
						fm[facet] = idx
						fo_bmatrix << facet << std::endl;
						++idx
						return *this;
					}

					Cell_handle c;
					if (CGAL::assign(c, input))
					{
					}

					throw SeriousError;
				}
		};

		static void write_persistence(
			ptr<Alpha_shape> as,
			std::string const &fn_bmatrix,
			std::string const &fn_points)
		{
			std::ofstream fo_bmatrix(fn_bmatrix), fo_points(fn_points);
			as->filtration(Phat_writer(fo_bmatrix, fo_points));
			fo_bmatrix.close(); fo_points.close();
		}
};

