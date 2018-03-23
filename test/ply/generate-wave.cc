#include "generate-wave.hh"

#include "base/unittest.hh"
#include "base/mvector.hh"
#include "base/mdrange.hh"
#include "base/map.hh"

using namespace System;


void generate_wave(PLY::PLY &ply, int N)
{
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
		PLY::list_property<unsigned, unsigned char>("vertex_index"));
	mVector<int,2> dx({0, 1}), dy({1, 0});
	for (auto p : MdRange<2>(mVector<int,2>(N-1)))
	{
		std::vector<int>
            v1 = {
                p[0] +     N * p[1],
                p[0] + 1 + N * p[1],
                p[0] + 1 + N * p[1] + N
            },
            v2 = {
                p[0] + 1 + N * p[1] + N,
                p[0] +     N * p[1] + N,
                p[0] +     N * p[1]             
            };
		ply.put_data(v1);
		ply.put_data(v2);
	}
}