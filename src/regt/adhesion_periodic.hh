#pragma once

#include <memory>

// CGAL definitions =================================
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Periodic_3_regular_triangulation_traits_3.h>
#include <CGAL/Periodic_3_regular_triangulation_3.h>

#include "system.hh"
#include "adhesion.hh"

namespace Conan
{
    using System::Header;
    using System::History;

    template<>
    class Adhesion_base<3, Periodic>
    {
    public:
        enum { R = 3 };

        //typedef CGAL::Cartesian<double>     K;
        typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
        typedef CGAL::Periodic_3_regular_triangulation_traits_3<K>  Gt;
        typedef CGAL::Periodic_3_regular_triangulation_3<Gt>        RT;

        typedef RT::Bare_point        Point;
        typedef RT::Edge              Edge;
        //typedef RT::Vector        Vector;
        typedef RT::Iso_cuboid        Iso_cuboid;
        typedef RT::Weighted_point    Weighted_point;
        typedef RT::Segment           Segment;
        typedef RT::Tetrahedron       Tetrahedron;

        static Point dVector2Point(dVector<3> const &p)
        { return Point(p[0], p[1], p[2]); }

        static dVector<R> Point2dVector(Point const &p)
        { return dVector<3>({p.x(), p.y(), p.z()}); }

        template <typename F>
        static Point make_Point(F f) { return Point(f(0), f(1), f(2)); }

        typedef RT::Cell_handle        Node;

    protected:
        System::ptr<System::Box<R>> box;
        System::ptr<RT> rt;

    public:
        Adhesion_base(BoxPtr<R> box_)
            : box(box_)
            , rt(new RT(Iso_cuboid(0, 0, 0, box->L(), box->L(), box->L())))
        {}

        int face_cnt(RT::Cell_handle h) const
        {
            int cnt = 0;
            for (unsigned i = 1; i < 4; ++i)
            {
                for (unsigned j = 0; j < i; ++j)
                {
                    if (rt->periodic_segment(h, i, j).squared_length()
                        / box->scale2() > 3.0)
                        ++cnt;
                }
            }

            return cnt;
        }

        double measure(Node h) const
        {
            return rt->periodic_tetrahedron(h).volume();
        }

        Weighted_point point(Node n, int i) const
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

}
