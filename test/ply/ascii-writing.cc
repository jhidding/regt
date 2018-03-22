#include <gtest/gtest.h>
#include "support/ply/ply.hh"

#include "base/unittest.hh"
#include "base/mvector.hh"
#include "base/mdrange.hh"
#include "base/map.hh"

using namespace System;

TEST(Ply, WritingASCII)
{
	PLY::PLY ply(PLY::ASCII);
	int N = 50;

	auto grid = map(MdRange<2>(mVector<int,2>(N)),
		[N] (mVector<int,2> const &X) { 
		return mVector<double,2>(X) * (2./N) - mVector<double,2>(1.0);
	});

	auto func =	[] (mVector<double,2> const &X) {
		double v = X.norm();
		return (v == 0 ? 1.0 : sin(v * 4*M_PI) / (v * 4*M_PI));
	};

	ply.comment("file written as test to DMT3D .ply writer");
	ply.add_element("vertex", 
		PLY::property<float>("x"), 
		PLY::property<float>("y"), 
		PLY::property<float>("z"),
		PLY::property<uint8_t>("red"),
		PLY::property<uint8_t>("green"),
		PLY::property<uint8_t>("blue")
	);
	for (auto p : grid)
	{
		double v = p.norm();
		auto q = p/v;
		double phase = atan2(q[0], q[1]);
		if (phase < 0) phase += 2*M_PI;
		if (phase > 2*M_PI) phase -= 2*M_PI;

		ply.put_data(
            p[0], p[1], func(p),
			(phase > M_PI ? 
				0 : 1.0 - 2 * fabs(phase/M_PI - 0.5))*255,
			((phase > (5./3 * M_PI)) or (phase < (2./3*M_PI)) ? 
				0 : 1.0 - 2 * fabs(phase/M_PI - 1.1667))*255,
			((phase > (1./3 * M_PI)) and (phase < (4./3*M_PI)) ? 
				0 : 1.0 - 2 * fabs((phase < (1./3 * M_PI) ? phase + 2*M_PI : phase)/M_PI - 1.8333))*255
		);
	}

	ply.add_element("face",
		PLY::list_property<unsigned, unsigned>("vertex_index"));
	mVector<int,2> dx({0, 1}), dy({1, 0});
	for (auto p : MdRange<2>(mVector<int,2>(N-1)))
	{
		std::vector<unsigned> v1, v2;
		v1.push_back(p[0] +     N * p[1]);
		v1.push_back(p[0] + 1 + N * p[1]);
		v1.push_back(p[0] + 1 + N * p[1] + N);
		ply.put_data(v1);
		v2.push_back(p[0] + 1 + N * p[1] + N);
		v2.push_back(p[0] +     N * p[1] + N);
		v2.push_back(p[0] +     N * p[1]);
		ply.put_data(v2);
	}

    std::ofstream f("ply_test.ply");
    f << ply;
    f.close();
}