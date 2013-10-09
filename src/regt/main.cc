#include "../base/system.hh"
#include "../base/format.hh"

#include <fstream>

using namespace System;

template <int R>
void regular_triangulation(std::ostream &fo, Header const &H, Array<double> phi)
{
	auto box = make_ptr<BoxConfig<R>>(H.get<unsigned>("mbits"), H.get<float>("L"));
	double t = H.get<double>("time");

	//==================================================
	// compute the Regular Triangulation
	//==================================================
	std::cerr << "creating triangulation ... ";
	auto adh = compute_adhesion<R>(box, t, phi);
	std::cerr << "[done]\n";

	cout << "writing needed info ... ";
	write_adhesion<R>(fo, adh);
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
		
		Option({Option::VALUED | Option::CHECK, "t", "time", "1.0",
			"growing mode parameter."}));

	if (C.get<bool>("help"))
	{
		std::cout << "Cosmic workset Conan, by Johan Hidding.\n\n";
		C.print(std::cout);
		exit(0);
	}

	std::string fn_input = timed_filename(argv["id"], "density", -1);
	std::string fn_output = timed_filename(argv["id"], "regt", -1);

	// read initial potential from file.
	std::ifstream fi;
	std::cerr << "reading " << fn_input << " ...\n";
	fi.open(fn_input.c_str(), std::ios::in | std::ios::binary);
	System::Header 	H(fi);
	System::History I(fi);
	Array<double> potential = load_from_file<double>(fi, "potential");
	fi.close();

	// add current command to history.
	I << argv;

	// write headers to file.
	std::ofstream fo;
	fo.open(fn_output.c_str(), std::ios::out | std::ios::binary);
	H.to_file(fo);
	I.to_file(fo);

	// give some reassuring output.
	std::cerr << "box: " << box->N() << " data: " << potential.size() << std::endl;

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

