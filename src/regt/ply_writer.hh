#pragma once
#include "adhesion.hh"

namespace Conan
{
	template <typename Adh>
	class VoronoiMap
	{
		using Point = typename Adh::Point;
        using Weighted_point = typename Adh::Weighted_point;
		using RT    = typename Adh::RT;
		using Node  = typename Adh::Node;

		std::map<Node, unsigned> V_map;
		std::vector<Point>  _vertices;
		std::vector<double> _mass;

		BoxPtr<3> box;
		ptr<RT>   rt;

		public:
			VoronoiMap(BoxPtr<3> box_, ptr<RT> rt_):
				box(box_), rt(rt_)
			{}

			unsigned operator()(Node h)
			{
				if (V_map.count(h) == 0)
				{
					unsigned q = _vertices.size();
					V_map[h] = q;

					_vertices.push_back(rt->dual(h));
					_mass.push_back(rt->tetrahedron(h).volume());
					return q;
				}
				else
				{
					return V_map[h];
				}
			}

			bool ok(Node h)
			{
				if (rt->is_infinite(h)) return false;
				for (unsigned k = 0; k < 4; ++k)
				{
					Weighted_point p = rt->point(h, k);
					for (unsigned k = 0; k < 3; ++k)
						if ((p[k] > box->L()) or (p[k] < 0))
							return false;
				}

				return true;
			}

			std::vector<Point> const &vertices() const
			{ return _vertices; }
	};

	template <typename Base>
	class PLY_writer: public Base
	{
		public:
			using Base::R;
			using Point = typename Base::Point;
			using RT    = typename Base::RT;

		protected:
			using Base::rt;
			using Base::box;

		public:
			PLY_writer(BoxPtr<R> box_):
				Base(box_)
			{
				if (R != 3) throw "PLY is only available in 3D";
			}

            virtual ~PLY_writer() {}

			virtual void save_all(Header const &H)
			{
				double t = H.get<double>("time");
				std::ostringstream ss;
				ss << std::setfill('0') << std::setw(5) << static_cast<int>(round(t * 10000));

				std::string fn_walls = Misc::format(H["new-id"], ".walls.", ss.str(), ".ply"),
					    fn_filam = Misc::format(H["new-id"], ".filam.", ss.str(), ".ply");

				write_filam_to_ply(fn_filam, H.get<double>("minli-fila"));
				write_walls_to_ply(fn_walls, H.get<double>("minli-wall"));

				Base::save_all(H);
			}

			void write_filam_to_ply(std::string const &filename, double minli) const
			{
				VoronoiMap<Base> cell_dual(box, rt);
				std::vector<std::pair<Array<unsigned>,double>> W;

				std::for_each(
					rt->finite_facets_begin(),
					rt->finite_facets_end(),
					[&] (typename RT::Facet const &f)
				{
					double l = rt->triangle(f).squared_area();
					if (l / (box->scale2()*box->scale2()) < 6.0) return;
					if (l < minli) return;

					Array<unsigned> P(0);
					auto c1 = f.first, c2 = rt->mirror_facet(f).first;
					if (not (cell_dual.ok(c1) and cell_dual.ok(c2))) return;
					if (Base::face_cnt(c1) < 5 and Base::face_cnt(c2) < 5) return;

					P->push_back(cell_dual(c1));
					P->push_back(cell_dual(c2));

					W.push_back(std::pair<Array<unsigned>,double>(P, l));
				});

				PLY::PLY ply;
				ply.comment("Adhesion model, filament component.");

				ply.add_element("vertex",
					PLY::property<float>("x"),
					PLY::property<float>("y"),
					PLY::property<float>("z"));
				for (Point const &v : cell_dual.vertices())
					ply.put_data(v[0], v[1], v[2]);

				ply.add_element("edge",
					PLY::property<int>("vertex1"),
					PLY::property<int>("vertex2"),
					PLY::property<float>("density"));
				for (auto f : W)
					ply.put_data((*f.first)[0], (*f.first)[1], f.second);

				ply.save(filename);
			}

			void write_walls_to_ply(std::string const &filename, double minli) const
			{
				VoronoiMap<Base> cell_dual(box, rt);
				std::vector<std::pair<Array<unsigned>,double>> W;

				std::for_each(
					rt->finite_edges_begin(),
					rt->finite_edges_end(),
					[&] (typename RT::Edge const &e)
				{
					double l = rt->segment(e).squared_length();
					if (l / box->scale2() < 4.0) return;
					if (l < minli) return;

					Array<unsigned> P(0);
					auto cells = rt->incident_cells(e), c = cells; ++c;
					if (cell_dual.ok(cells))
						P->push_back(cell_dual(cells));

					for (; c != cells; ++c)
						if (cell_dual.ok(c))
							P->push_back(cell_dual(c));

					W.push_back(std::pair<Array<unsigned>,double>(P, l));
				});

				PLY::PLY ply;
				ply.comment("Adhesion model, wall component.");

				ply.add_element("vertex",
					PLY::property<float>("x"),
					PLY::property<float>("y"),
					PLY::property<float>("z"));
				for (Point const &v : cell_dual.vertices())
					ply.put_data(v[0], v[1], v[2]);

				ply.add_element("face",
					PLY::list_property<int, uint8_t>("vertex_index"),
					PLY::property<float>("density"));
				for (auto f : W)
					if (f.first.size() > 2)
					    ply.put_data(*f.first, f.second);

				ply.save(filename);
			}
	};
}

