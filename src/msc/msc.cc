#include "../base/system.hh"
#include "../base/format.hh"
#include "../base/boxconfig.hh"

#include "../misc/gradient.hh"
#include "msc.hh"

using namespace System;

void command_msc(int argc, char **argv)
{
	Argv args = read_arguments(argc, argv,
		Option(0, "h", "help", "false", "shows this help."),

		Option(Option::VALUED | Option::CHECK, "i", "id", date_string(), 
			"Identifier of the run. This tag is prefixed to all input "
			"and output filenames. By default the current date is used. "));
		
	if (args.get<bool>("help"))
	{
		std::cerr << "DMT3D - catastrophes at your doorstep.\n"
			"Copyright Johan Hidding, June 2013 - licence: GPL3.\n\n"
			"This command computes the Morse-Smale complex, it's here for \n"
			"testing purposes mainly.\n";
		args.print(std::cerr);
		exit(0);
	}

	std::ifstream fi(Misc::format(args["id"], ".init.conan"));
	Header H(fi); History I(fi);
	unsigned bits = H.get<unsigned>("mbits");
	double L = H.get<double>("size");
	Array<double> density = load_from_file<double>(fi, "density");

	switch (H.get<unsigned>("dim"))
	{
		case 2: {
			auto box = make_ptr<BoxConfig<2>>(bits, L);
			Misc::Gradient<2> G(box, density);
			for (size_t i = 0; i < box->size(); ++i)
			{
				if (G.root_potentially_within_cell(i))
					std::cout << box->box().dvec(i) << std::endl;
			}

			std::cout << "\n\n\n";
			DMT::MSC<2> msc(bits, density);
			msc.generate_gradient();
			msc.print_critical_points(std::cout); 
			
			std::cout << "\n\n\n";
			for (size_t i = 0; i < box->size(); ++i)
			{
				std::cout << box->box().dvec(i) << " " << G[i] << " " << G[i][0] * G[i][1] << std::endl;
			}
		} break;

		case 3: { DMT::MSC<3> msc(bits, density);
			msc.generate_gradient();
			msc.print_critical_points(std::cout); } break;
	}
}

Global<Command> _COMMAND_MSC("msc", command_msc);

