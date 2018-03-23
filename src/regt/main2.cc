#include "../base/system.hh"
#include "../base/format.hh"
#include "adhesion2.hh"
#include "velocity.hh"
#include "ply_writer.hh"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace System;
using namespace Conan;

template <unsigned R>
ptr<Adhesion<R>> make_adhesion(Header const &H);

template <>
ptr<Adhesion<2>> make_adhesion<2>(Header const &H)
{
	auto box = make_ptr<Box<2>>(H.get<unsigned>("N"), H.get<float>("size"));
	return make_ptr<Velocity<Adhesion<2>>>(box);
}

template <>
ptr<Adhesion<3>> make_adhesion<3>(Header const &H)
{
	auto box = make_ptr<Box<3>>(H.get<unsigned>("N"), H.get<float>("size"));	
	if (H.get<bool>("ply"))
	{
		return make_ptr<PLY_writer<Velocity<Adhesion<3>>>>(box);
	}
	else
	{
		return make_ptr<Velocity<Adhesion<3>>>(box);
	}
}

template <unsigned R>
ptr<Adhesion<R>> make_adhesion2(Header const &H, Array<double> phi)
{
	ptr<Adhesion<R>> adh = make_adhesion<R>(H);
	double t = H.get<double>("time");

	if (H.get<bool>("glass"))
	{
		std::string fn_glass = timed_filename(H["id"], "glass", -1);
		std::cerr << "reading glass ... " << fn_glass << "\n";
		std::ifstream fi(fn_glass);
		System::Header 	gH(fi);
		System::History gI(fi);

		Array<mVector<double,R>> glass(fi);
		std::cerr << "creating triangulation ... ";
		adh->from_potential_with_glass(glass, phi, t);
		std::cerr << "[done]\n";
		return adh;
	} 
	else
	{ 
		std::cerr << "creating triangulation ... ";
		adh->from_potential(phi, t);
		std::cerr << "[done]\n";
		return adh;
	} 
}

template <unsigned R>
void regular_triangulation2(std::ostream &fo, Header const &H, Array<double> phi)
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

	auto adh = make_adhesion2<R>(H, phi);

	std::cerr << "writing needed info ... ";
	adh->save_all(H);
}

void cmd_regt2(int argc, char **argv)
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

		Option({0, "txt", "txt", "false",
			"save data as text; default is binary."}),

		Option({0, "ply", "ply", "false",
			"write data to PLY, only for 3D."}),

		Option({Option::VALUED | Option::CHECK, "", "minli-wall", "0",
			"minimal Lagrangian interval to store, a higher value "
			"reduces size of files written. Number is length."}),
		
		Option({Option::VALUED | Option::CHECK, "", "minli-fila", "0",
			"minimal Lagrangian interval to store, a higher value "
			"reduces size of files written. Number is area."}),
		
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
		H["new-id"] = ss.str();
	}
	else
	{
		H["new-id"] = H["id"];
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
		case 2: regular_triangulation2<2>(fo, H, potential);
			break;

		case 3: regular_triangulation2<3>(fo, H, potential);
			break;
	}

	fo.close();
}

Global<Command> _REGT2("adhesion", cmd_regt2);

