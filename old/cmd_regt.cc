#include <complex>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <set>

#include "../libconan/getopt/getopt.h"
#include "../libconan/default_filename.h"
#include "../libconan/dft/dft.h"
#include "../libconan/complex_util.h"

#include "../libconan/spaces.h"
#include "../libconan/cvector.h"
#include "../libconan/conan.h"
#include "../libconan/datastream.h"

#include "adhesion.h"

using namespace std;
using namespace Conan;

template <int R>
void build_output(std::shared_ptr<Adhesion<R>> adh, std::ostream &fo)
{}

template <>
void build_output<2>(std::shared_ptr<Adhesion<2>> adh, std::ostream &fo)
{
	constexpr int rank = 2;
	fo << "# power diagram" << endl; cout.flush();
	adh->for_each_big_dual_segment(
		[&fo] (typename Adhesion<rank>::Segment const &s, double r)
	{
		typename Adhesion<rank>::Point a = s.start(), b = s.end();
		fo 	<< a[0] << " " << a[1] << " " << r << "\n" 
			<< b[0] << " " << b[1] << " " << r << "\n\n\n";
	});

//	fo << endl << endl << "# regular triangulation" << endl;
//	adh->for_each_segment(
//		[&fo] (typename Adhesion<rank>::Segment const &s)
//	{
//		typename Adhesion<rank>::Point a = s.start(), b = s.end();
//		fo 	<< a[0] << " " << a[1] << " " << b[0] << " " << b[1] << " " << endl;	
//	});
//
//	fo << endl << endl << "# big triangles" << endl;
//	adh->for_each_big_triangle(
//		[&fo] (typename Adhesion<rank>::Triangle const &t, int i)
//	{
//		for (unsigned j = 0; j < 3; ++j)
//			fo << t[j][0] << " " << t[j][1] << " ";
//		fo << i << endl;
//	});
//
//	fo << endl << endl << "# clusters" << endl;
//	adh->for_each_cluster(
//		[&fo] (typename Adhesion<rank>::Point const &p, double r)
//	{
//		fo << p[0] << " " << p[1] << " " << r << endl;
//	});
//	cout << "[done]" << endl; cout.flush();
}

template <>
void build_output<3>(std::shared_ptr<Adhesion<3>> adh, std::ostream &fo)
{
	constexpr int rank = 3;
	fo << "# segments" << endl;
	adh->for_each_big_dual_segment(
		[&fo] (typename Adhesion<rank>::Segment const &s, double r)
	{
		typename Adhesion<rank>::Point a = s.start(), b = s.end();
		fo 	<< a << " " << b << " " << r << endl;
	});

	fo << "# nodes" << endl;
	adh->for_each_cluster(
		[&fo] (typename Adhesion<rank>::Point const &p, double m)
	{
		fo	<< p << " " << m << endl;
	});
}

template <int rank>
void regular_triangulation(std::istream &fi, std::ostream &fo)
{
	GetOpt options = GetOpt::instance();
	size_t 		size 	= BoxConfig::size();
	unsigned	N 	= BoxConfig::N();
	//unsigned	bits	= BoxConfig::bits();
	float		L	= BoxConfig::L();

	float b = options.get_value("-b", 1.0);

	//==================================================
	// calculate the potential by Fourier method
	//==================================================
	cout << "calculating potential ... ";
	Cube<double>	*RF = new Cube<double>(fi);
	std::shared_ptr<Cube<double>>	phi(new Cube<double>);

	DFT 			dft(rank, BoxConfig::bits());
	Spaces::KSpace<rank>	K(N, L);

	copy(RF->begin(), RF->end(), dft.in());
	dft.forward();

	transform(K.begin(), K.end(), dft.out(), dft.in(), [] 
		(mVector<float, rank> const &k, complex<double> const &v)
	{ 
		return v / sqr(k); 
	});

	dft.in()[0] = 0;
	dft.backward();

	transform(dft.out(), dft.out() + size, phi->begin(), get_real);
	transform(phi->begin(), phi->end(), phi->begin(), [size, b] (double f)
	{ 
		return 2 * b * f / double(size); 
	});

	delete RF;
	cout << "[done]\n";

	//==================================================
	// compute the Regular Triangulation
	//==================================================
	cout << "creating triangulation ... "; cout.flush();
	auto adh = Adhesion<rank>::create(phi);
	cout << "[done]" << endl; cout.flush();

	cout << "writing needed info ... ";
	build_output<rank>(adh, fo);
}

void regular_triangulation()
{
	GetOpt options = GetOpt::instance();

	string fn_input = options.get_value("-input",
			timed_filename("delta0"));

	float b = options.get_value("-b", 1.0);

	string fn_output = options.get_value("-output",
			timed_filename("rt", b));

	ifstream fi;
	fi.open(fn_input.c_str(), ios::in | ios::binary);

	Header 		H(fi);
	History 	I(fi);

	unsigned	rank 	= from_string<unsigned>(H["rank"]);
	unsigned	N	= from_string<unsigned>(H["N"]);
	float		L 	= from_string<float>(H["L"]);

	BoxConfig::initialize(rank, N, L);

	ofstream fo;
	fo.open(fn_output.c_str(), ios::out | ios::binary);

	//H.to_file(fo);
	//I.to_file(fo);
	
	switch (rank)
	{
		case 2: regular_triangulation<2>(fi, fo); break;
		case 3: regular_triangulation<3>(fi, fo); break;
	}

	fi.close();
	fo.close();
}

