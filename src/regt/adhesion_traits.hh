#pragma once

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Regular_triangulation_3.h>
#include <CGAL/Regular_triangulation_2.h>
#include <CGAL/Periodic_3_regular_triangulation_traits_3.h>
#include <CGAL/Periodic_3_regular_triangulation_3.h>

#include "system.hh"

namespace Conan
{
    using System::Header;
    using System::History;

    /*! Used in templates to pick the correct regular triangulation
     *  algorithm.
     */
    enum BoxTopology { NonPeriodic, Periodic };

    /*! Variants of methods to be called on the different regular
     *  triangulation classes in CGAL.
     */
    template <unsigned R, BoxTopology Topology>
    class Adhesion_traits;

    /*! Traits class for 2D non-periodic adhesion model.
     */
    template <>
    class Adhesion_traits<2, NonPeriodic>
    {
    public:
        constexpr static unsigned R = 2;

        using K  = CGAL::Exact_predicates_inexact_constructions_kernel;
        using RT = CGAL::Regular_triangulation_2<K>;

        /*! Get the area of a face.
         */
        static double measure(RT const &rt, RT::Face_handle f)
        {
            return rt.triangle(f).area();
        }

        /*! Get the length of an edge.
         */
        static double measure(RT const &rt, RT::Edge const &e)
        {
            return sqrt(rt.segment(e).squared_length());
        }

        /*! Construct the empty regular triangulation object.
         */
        static std::unique_ptr<RT> make_rt(BoxPtr<R> box)
        {
            return std::make_unique<RT>();
        }
    };

    /*! Traits class for 3D non-periodic adhesion model.
     */
    template <>
    class Adhesion_traits<3, NonPeriodic>
    {
    public:
        constexpr static unsigned R = 3;

        using K  = CGAL::Exact_predicates_inexact_constructions_kernel;
        using RT = CGAL::Regular_triangulation_3<K>;

        /*! Get the length of an edge.
         */
        static double measure(RT const &rt, RT::Edge const &e)
        {
            return sqrt(rt.segment(e).squared_length());
        }

        /*! Get the area of a face.
         */
        static double measure(RT const &rt, RT::Facet const &f)
        {
            return sqrt(rt.triangle(f).squared_area());
        }

        /*! Get the volume of a cell.
         */
        static double measure(RT const &rt, RT::Cell_handle c)
        {
            return rt.tetrahredon(c).volume();
        }

        /*! Construct the empty regular triangulation object.
         */
        static std::unique_ptr<RT> make_rt(BoxPtr<R> box)
        {
            return std::make_unique<RT>();
        }
    };

    /*! Traits class for 3D non-periodic adhesion model.
     */
    template <>
    class Adhesion_traits<3, Periodic>
    {
    public:
        constexpr static unsigned R = 3;

        using K  = CGAL::Exact_predicates_inexact_constructions_kernel;
        using Gt = CGAL::Periodic_3_regular_triangulation_traits_3<K>;
        using RT = CGAL::Periodic_3_regular_triangulation_3<Gt>;

        /*! Get the length of an edge.
         */
        static double measure(RT const &rt, RT::Edge const &e)
        {
            return sqrt(rt.construct_segment(e).squared_length());
        }

        /*! Get the area of a face.
         */
        static double measure(RT const &rt, RT::Facet const &f)
        {
            return sqrt(rt.construct_triangle(f).squared_area());
        }

        /*! Get the volume of a cell.
         */
        static double measure(RT const &rt, RT::Cell_handle c)
        {
            return rt.construct_tetrahredon(c).volume();
        }

        /*! Construct the empty regular triangulation object.
         */
        static std::unique_ptr<RT> make_rt(BoxPtr<R> box)
        {
            return std::make_unique<RT>
                (RT::Iso_cuboid(0, 0, 0, box->L(), box->L(), box->L()));
        }
    };
}
