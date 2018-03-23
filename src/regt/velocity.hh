#pragma once
#include "adhesion.hh"

namespace Conan
{
	inline dVector<3> cross_3(dVector<3> const &a, dVector<3> const &b)
	{
		return dVector<3>({ a[1]*b[2] - a[2]*b[1],
				    a[2]*b[0] - a[0]*b[2],
				    a[0]*b[1] - a[1]*b[0] });
	}

	inline dVector<4> cross_4(dVector<4> const &a, dVector<4> const &b, dVector<4> const &c)
	{
		return dVector<4>({ - a[1] * (b[2]*c[3] - b[3]*c[2])
				    - a[2] * (b[3]*c[1] - b[1]*c[3])
				    - a[3] * (b[1]*c[2] - b[2]*c[1]),

				      a[2] * (b[3]*c[0] - b[0]*c[3])
				    + a[3] * (b[0]*c[2] - b[2]*c[0])
				    + a[0] * (b[2]*c[3] - b[3]*c[2]),

				    - a[3] * (b[0]*c[1] - b[1]*c[0])
				    - a[0] * (b[1]*c[3] - b[3]*c[1])
				    - a[1] * (b[3]*c[0] - b[0]*c[3]),

				      a[0] * (b[1]*c[2] - b[2]*c[1])
				    + a[1] * (b[2]*c[0] - b[0]*c[2])
				    + a[2] * (b[0]*c[1] - b[1]*c[0]) });
	}

	template <unsigned R>
	class Velocity_base;

	template <>
	class Velocity_base<2>
	{
		typedef Adhesion_base<2> A;

		public:
			static dVector<3> normal(A::Point const *p, double const *w)
			{
				dVector<3> b, c;
				b[0] = p[1].x() - p[0].x();
				b[1] = p[1].y() - p[0].y();
				b[2] = w[1]     - w[0];

				c[0] = p[2].x() - p[0].x();
				c[1] = p[2].y() - p[0].y();
				c[2] = w[2]     - w[0];

				return cross_3(b, c);
			}

			static dVector<2> gradient(dVector<3> const &n)
			{
				if ((n[0] == 0) and (n[1] == 0))
					return dVector<2>(0.0);

				double f = -1./n[2];

				return dVector<2>({f*n[0], f*n[1]});
			}

	};

	template <>
	class Velocity_base<3>
	{
		typedef Adhesion_base<3> A;

		public:
			static dVector<4> normal(A::Point const *p, double const *w)
			{
				dVector<4> b, c, d;
				b[0] = p[1].x() - p[0].x();
				b[1] = p[1].y() - p[0].y();
				b[2] = p[1].z() - p[0].z();
				b[3] = w[1]     - w[0];

				c[0] = p[2].x() - p[0].x();
				c[1] = p[2].y() - p[0].y();
				c[2] = p[2].z() - p[0].z();
				c[3] = w[2]     - w[0];

				d[0] = p[3].x() - p[0].x();
				d[1] = p[3].y() - p[0].y();
				d[2] = p[3].z() - p[0].z();
				d[3] = w[3]     - w[0];

				return cross_4(b, c, d);
			}

			static dVector<3> gradient(dVector<4> const &n)
			{
				if ((n[0] == 0) and (n[1] == 0) and (n[2] == 0))
					return dVector<3>(0.0);

				double sign = (n[3] < 0 ? -1.0 : 1.0);
				double f = -sign/n[3];

				return dVector<3>({f*n[0], f*n[1], f*n[2]});
			}
	};

	template <unsigned R>
	struct VelocityInfo
	{
		dVector<R> x, v;
		double     mass;
		int	   type;
	};

	template <typename Base>
	class Velocity: public Base
	{
		public:
			using Base::R;
			using RT 	= typename Base::RT;
			using Point 	= typename Base::Point;
			using Node 	= typename Base::Node;
			using Base::rt;
			using Base::box;

			typedef Velocity_base<R> V;

			Velocity(BoxPtr<R> box):
				Base(box)
			{
				// a suitable typname for unpacking with Python.numpy
				std::string type_name = Misc::format("["
					"('pos','", System::TypeRegister::name<dVector<R>>(), "'),",
					"('vel','", System::TypeRegister::name<dVector<R>>(), "'),",
					"('mass','f8'),('type','i4')]");

				System::TypeRegister::set_name<VelocityInfo<R>>(type_name);
			}

            virtual ~Velocity() {}

			dVector<R> velocity(Node c, double t = 1.0)
			{
				Point	points[R+1];
				double	weights[R+1];

				for (unsigned i = 0; i < R+1; ++i)
				{
					typename Base::Weighted_point pt = Base::point(c, i);
					points[i] = pt.point();
					weights[i] = pt.weight();
				}

				return - V::gradient(V::normal(points, weights)) / (2*t);
			}

			void save_nodes_txt(std::ostream &fo, double t)
			{
				Base::for_each_node([&] (Node i)
				{
					fo << rt->dual(i) << " " << velocity(i, t) << " "
					   << Base::face_cnt(i) << " " << Base::measure(i) << std::endl;
				});
			}

			void save_nodes_binary(std::ostream &fo, double t)
			{
				Array<VelocityInfo<R>> data;
				Base::for_each_node([&] (Node i)
				{
					data->push_back({
						Base::Point2dVector(rt->dual(i)),
						velocity(i, t),
						Base::measure(i), Base::face_cnt(i)});
				});

				save_to_file(fo, data, "nodes");
			}

			virtual void save_all(Header const &H)
			{
				double t = H.get<double>("time");
				std::ostringstream ss;
				ss << std::setfill('0') << std::setw(5) << static_cast<int>(round(t * 10000));

				std::string fn = Misc::format(H["new-id"], ".nodes.", ss.str(), ".conan");
				std::ofstream fo(fn);

				if (H.get<bool>("txt"))
				{
					save_nodes_txt(fo, t);
				}
				else
				{
					H.to_file(fo);
					History I; I.update("<adhesion code>"); I.to_file(fo);
					save_nodes_binary(fo, t);
				}
			}
	};
}

