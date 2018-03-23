#pragma once
#include "../base/system.hh"
#include "../ply/ply.hh"
#include "../misc/interpol.hh"

namespace Conan
{
	template <unsigned R>
	using BoxPtr = System::ptr<System::Box<R>>;

	template <unsigned R>
	using dVector = System::mVector<double, R>;

	template <unsigned R>
	using iVector = System::mVector<int, R>;

	using System::mVector;
	using System::Array;
	using System::ptr;
	using System::Box;
	using Misc::PLY;
}

