#pragma once
#include <png++/png.hpp>
#include "../numeric/ndarray.hh"
#include <experimental/tuple>

namespace eScatter
{
    using std::experimental::apply;

    template <typename T>
    void write_png(numeric::NdArray<T, 2> const &data, std::string const &filename)
    {
        auto shape = data.shape();

        T min = data[0], max = data[0];
        for (T value : data.data())
        {
            min = std::min(value, min);
            max = std::max(value, max);
        }

        auto palette = [min, max] (T value)
        {
            T x = (value - min) / (max - min);
            T r = 0.237 - 2.13*x + 26.92*pow(x,2) - 65.5*pow(x,3) + 63.5*pow(x,4) - 22.36*pow(x,5),
              g = pow((0.572 + 1.524*x - 1.811*pow(x,2))/(1 - 0.291*x + 0.1574*pow(x,2)), 2),
              b = 1./(1.579 - 4.03*x + 12.92*pow(x,2) - 31.4*pow(x,3) + 48.6*pow(x,4) - 23.36*pow(x,5));
            return std::make_tuple<uint16_t,uint16_t,uint16_t>(r*65535, g*65535, b*65535);
        };

        png::image<png::rgb_pixel_16> image(shape[0], shape[1]);
        size_t idx = 0;
        for (unsigned j = 0; j < shape[1]; ++j)
            for (unsigned i = 0; i < shape[0]; ++i, ++idx)
                image[j][i] = apply([] (uint16_t r, uint16_t g, uint16_t b)
                    { return png::rgb_pixel_16(r, g, b); }, palette(data[idx]));

        image.write(filename);
    }
}

