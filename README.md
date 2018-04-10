
# Table of Contents

1.  [Regular triangulations for Cosmology](#orga056299)
    1.  [Features](#org30fcbbe)
        1.  [future](#org8659365)
    2.  [Output](#org5d27ab3)
    3.  [Build](#org22f9977)
        1.  [Prerequisites](#orge2b2c3a)
        2.  [Building](#org2ad643b)


<a id="orga056299"></a>

# Regular triangulations for Cosmology

Regular triangulations are a very useful tool compute the **geometric
adhesion model**.  This code solves Burgers' equation

\[\frac{\partial u}{\partial t} + (u \cdot \grad)u = \nu \grad^2 u.\]

This equation is solved using geometric methods available in the
Computational Geometry Algorithms Library (CGAL).


<a id="org30fcbbe"></a>

## Features

-   generate initial conditions
-   create glass files
-   compute Zeldovich displacements
-   compute the adhesion model
-   work in 2D or 3D
-   output in PLY


<a id="org8659365"></a>

### future

1.  (merge from git:jhidding/adhesion-example repo)

    -   OBJ triangle output (splits polygons into triangles, but renders better)
    -   Lloyd iteration glasses

2.  Todo

    -   periodic triangulations
    -   use HDF5
    -   increase test coverage


<a id="org5d27ab3"></a>

## Output

The output of the 3D adhesion code is in PLY format. PLY is a very liberal specification that
supports storing polygons of any number. Filaments and wall structures are stored in different
files (clusters are stored separately). We associate a density with each element in the PLY file.
This is within specification but not *canon*, so some applications claiming to support PLY
(like paraview) may crash trying to load these files.


<a id="org22f9977"></a>

## Build


<a id="orge2b2c3a"></a>

### Prerequisites

-   CGAL >= 4.11
-   C++17 compatible compiler (GCC >= 7, LLVM/clang >= 4)
-   Meson / ninja build system
-   FFTW3
-   GNU Scientific Library (GSL)
-   (for unit testing only) GTest


<a id="org2ad643b"></a>

### Building

In the project folder, run `meson` and subsequently `ninja`.

    $ meson build --buildtype release
    ...
    $ cd build
    build$ ninja

To run unit tests (currently only tests PLY output code)

    build $ ninja test

