/*
 * main interface to the convoluted adhesion code
 *
 * first argument should be a command word
 * the rest are options to this command
 */

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <sstream>

#include <algorithm>
#include <complex>

#include "../libconan/getopt/getopt.h"

extern void regular_triangulation();

using namespace std;

void usage(std::string const &name)
{
	cout 	<< "CONvoluted AdhesioN code" << endl;
	exit(1);
}


int main(int argc, char **argv)
{ 
	try 
	{
		GetOpt options = GetOpt::initialize(argc, argv);
		if (options.size() < 2) usage(options.name());
		string command = options[1];

		if (command == "run")
		{
			regular_triangulation();
			return 0;
		}

		usage(options.name());
		return 0;
	}

	catch (const char *a)
	{
		cout << a << endl;
	}

	catch (string const &s)
	{
		cout << s << endl;
	}
}

