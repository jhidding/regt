Regular triangulations for Cosmology {#sec:orgbf10868}
====================================

Regular triangulations are a very useful tool compute the **geometric
adhesion model**. This code solves Burgers’ equation

$$\frac{\partial u}{\partial t} + (u \cdot \grad)u = \nu \grad^2 u.$$

This equation is solved using geometric methods available in the
Computational Geometry Algorithms Library (CGAL).

Features {#sec:orgb3c7d53}
--------

-   generate initial conditions

-   create glass files

-   compute Zeldovich displacements

-   compute the adhesion model

-   work in 2D or 3D

-   output in PLY

### future {#sec:org48fa9d4}

1.  (merge from git:jhidding/adhesion-example repo) \[sec:orgf6612ee\]

    -   OBJ triangle output (splits polygons into triangles, but renders
        better)

    -   Lloyd iteration glasses

2.  Todo \[sec:org3579565\]

    -   periodic triangulations

    -   use HDF5

    -   increase test coverage

Output {#sec:org06f27f0}
------

The output of the 3D adhesion code is in PLY format. PLY is a very
liberal specification that supports storing polygons of any number.
Filaments and wall structures are stored in different files (clusters
are stored separately). We associate a density with each element in the
PLY file. This is within specification but not *canon*, so some
applications claiming to support PLY (like paraview) may crash trying to
load these files.

Build {#sec:orgb7951f2}
-----

### Prerequisites {#sec:orgf108ac3}

-   CGAL &gt;= 4.11

-   C++17 compatible compiler (GCC &gt;= 7, LLVM/clang &gt;= 4)

-   Meson / ninja build system

-   FFTW3

-   GNU Scientific Library (GSL)

-   (for unit testing only) GTest

### Building {#sec:orgf598aa9}

In the project folder, run `meson` and subsequently `ninja`.

    $ meson build --buildtype release
    ...
    $ cd build
    build$ ninja

To run unit tests (currently only tests PLY output code)

    build $ ninja test
