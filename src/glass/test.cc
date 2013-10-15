#ifdef UNITTEST
#include "distances.hh"
#include "../base/system.hh"
#include "../base/unittest.hh"

using namespace System;

typedef mVector<double, 2> Point;

class Printer: public kdTree::Visitor<Point>
{
	std::ostream &out;

	public:
		Printer(std::ostream &out_): out(out_) {}

		virtual void operator()(Point const &B)
		{
			out << B << std::endl;	
		}
};

Test::Unit kdTree_test("9827 - kdTree",
	"I use kdTrees to do a lot of things, but implementing the right "
	"predicates for periodic spaces can be tricky. This should help.",
	[] ()
{
	typedef mVector<double, 2> Point;

	std::ofstream fo("kdtree_test.dat");
	fo << "# kdtree test file\n";
	Array<Point> X(10000);

	generate(X, Glass::random_uniform_particles<2>(0, 100.0));
	kdTree::Tree<Point,2> T(X.begin(), X.end(), [] (Point const &p, int i) { return p[i]; });
	Printer P(fo);
	T.traverse(P, Glass::Annulus<2>(100, Point({20,30}), 35, 45));
	fo << "\n\n";
	T.traverse(P, Glass::Disc<2>(100, Point({70,80}), 40));
	fo.close();

	return true;
});

#endif

