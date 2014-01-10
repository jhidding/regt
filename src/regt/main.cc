#include "../base/system.hh"
#include "../base/format.hh"
#include "adhesion.hh"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace System;
using namespace Conan;

std::ostream &operator<<(std::ostream &out, typename Adhesion<2>::Point const &p)
{ return out << p[0] << " " << p[1]; }
std::ostream &operator<<(std::ostream &out, typename Adhesion<3>::Point const &p)
{ return out << p[0] << " " << p[1] << " " << p[2]; }

template <unsigned R>
void write_adhesion_txt(std::ostream &out, ptr<Adhesion<R>> adh)
{
	adh->for_each_big_dual_segment([&] (typename Adhesion<R>::Segment const &s, double v)
	{
		typename Adhesion<R>::Point a = s.start(), b = s.end();
		out << a << " " << v << "\n" << b << " " << v << "\n\n\n";
	});
}

template <unsigned R>
void write_adhesion_clusters(std::ostream &out, ptr<Adhesion<R>> adh, double L)
{
	adh->for_each_cluster([&] (typename Adhesion<R>::Point const &p, double r)
	{
		for (unsigned i = 0; i < R; ++i)
			if (p[i] > L or p[i] < 0) return;

		out << p << " " << r << std::endl;
	});
}

template <unsigned R>
ptr<Adhesion<R>> make_adhesion(Header const &H, Array<double> phi)
{
	auto box = make_ptr<Box<R>>(H.get<unsigned>("N"), H.get<float>("size"));
	double t = H.get<double>("time");

	if (H.get<bool>("glass"))
	{
		std::cerr << "reading glass ... \n";
		std::ifstream fi(timed_filename(H["id"], "glass", -1));
		System::Header 	gH(fi);
		System::History gI(fi);

		Array<mVector<double,R>> glass(fi);
		std::cerr << "creating triangulation ... ";
		auto adh = Adhesion<R>::create_from_glass(box, glass, phi, t);
		std::cerr << "[done]\n";
		return adh;
	} 
	else
	{ 
		std::cerr << "creating triangulation ... ";
		auto adh = Adhesion<R>::create(box, phi, t);
		std::cerr << "[done]\n";
		return adh;
	} 
}

template <unsigned R>
void regular_triangulation(std::ostream &fo, Header const &H, Array<double> phi)
{
	if (H["smooth"] != "0")
	{
		std::cerr << "Smoothing ... ";
		auto box = make_ptr<Box<R>>(H.get<unsigned>("N"), H.get<float>("size"));
		Fourier::Transform fft(std::vector<int>(R, box->N()));
		double sigma = H.get<double>("smooth");
		copy(phi, fft.in);
		auto S = Fourier::Fourier<R>::scale(sigma / box->scale());

		fft.forward();
		transform(fft.out, box->K, fft.in, Fourier::Fourier<R>::filter(S));
		fft.in[0] = 0;
		fft.backward();
		transform(fft.out, phi, Fourier::real_part(box->size()));
		std::cerr << "[done]\n";
	}

	double t = H.get<double>("time");
	auto adh = make_adhesion<R>(H, phi);

	std::cerr << "writing needed info ... ";
	std::ostringstream ss;
	ss << std::setfill('0') << std::setw(5) << static_cast<int>(round(t * 10000));
	std::string fn_ply = Misc::format(H["id"], ".", ss.str(), ".walls.ply"),
		fn_filply  = Misc::format(H["id"], ".", ss.str(), ".filam.ply"),
		fn_cluster = Misc::format(H["id"], ".", ss.str(), ".clust.txt"),
		fn_bmatrix = Misc::format(H["id"], ".", ss.str(), ".bmatrix.txt"),
		fn_points  = Misc::format(H["id"], ".", ss.str(), ".points.txt"),
		fn_values  = Misc::format(H["id"], ".", ss.str(), ".values.txt");

	std::ofstream fo_cluster(fn_cluster);
	switch (R)
	{
		case 2: write_adhesion_clusters<R>(fo_cluster, adh, H.get<double>("size")); break;
		case 3: write_adhesion_clusters<R>(fo_cluster, adh, H.get<double>("size"));
			write_adhesion_txt<R>(fo, adh);
			//adh->walls_to_ply_file(fn_ply);
			//adh->filam_to_ply_file(fn_filply);
			if (H.get<bool>("persistence"))
				adh->write_persistence(fn_bmatrix, fn_points, fn_values);

			break;
	}
}

void cmd_regt(int argc, char **argv)
{
	std::ostringstream ss;
	ss << time(NULL);
	std::string timed_seed = ss.str();

	Argv C = read_arguments(argc, argv,
		Option({0, "h", "help", "false",
			"print help on the use of this program."}),

		Option({Option::VALUED | Option::CHECK, "i", "id", date_string(),
			"identifier for filenames."}),

		Option({Option::VALUED | Option::CHECK, "s", "smooth", "0",
			"smooth the initial conditions before running adhesion. "
			"By default, no smoothing is done, otherwise this parameter "
			"is the value of sigma. "}),

		Option({0, "g", "glass", "false",
			"use a glass file in stead of a regular grid pattern. "
			"The file should be called <id>.glass.init.conan."}),

		Option({0, "p", "persistence", "false",
			"write the result in the form of persistence data, readable "
			"by the [phat] package."}),
		
		Option({Option::VALUED | Option::CHECK, "t", "time", "1.0",
			"growing mode parameter."}));

	if (C.get<bool>("help"))
	{
		std::cout << "Cosmic workset Conan, by Johan Hidding.\n\n";
		C.print(std::cout);
		exit(0);
	}

	std::string fn_input = timed_filename(C["id"], "density", -1);

	// read initial potential from file.
	std::ifstream fi;
	std::cerr << "reading " << fn_input << " ...\n";
	fi.open(fn_input.c_str(), std::ios::in | std::ios::binary);
	System::Header 	H(fi);
	System::History I(fi);
	Array<double> potential = load_from_file<double>(fi, "potential");
	fi.close();

	// add current command to history.
	H << C; I << C;
	if (H["smooth"] != "0")
	{
		std::ostringstream ss;
		ss << H["id"] << ".s" << H["smooth"];
		H["id"] = ss.str();
	}

	double t = H.get<double>("time");
	std::string fn_output = timed_filename(H["id"], "regt", t);

	// write headers to file.
	std::ofstream fo;
	fo.open(fn_output.c_str(), std::ios::out | std::ios::binary);
	H.to_txt_file(fo);
	I.to_txt_file(fo);

	// give some reassuring output.
	//std::cerr << "box: " << box->N() << " data: " << potential.size() << std::endl;

	// run 2 or 3 dimensional version.
	switch (H.get<unsigned>("dim"))
	{
		case 2: regular_triangulation<2>(fo, H, potential);
			break;

		case 3: regular_triangulation<3>(fo, H, potential);
			break;
	}

	fo.close();
}

Global<Command> _REGT("regt", cmd_regt);

