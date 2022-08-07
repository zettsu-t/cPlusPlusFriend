#ifndef JULIASET_H
#define JULIASET_H

#include <cmath>
#include <cstdint>
#include <complex>
#include <filesystem>
#include <optional>
#include <string>
#include <thread>
#include <boost/multi_array.hpp>
#include <boost/gil.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/io/write_view.hpp>

/**
 C++ implementation
 */
using Count = int32_t;
using Coordinate = float;
using PixelSize = size_t;
using Point = std::complex<Coordinate>;
using CoordinateSet = boost::multi_array<Coordinate, 1>;
using CoordinateSetView = typename CoordinateSet::array_view<1>::type;
using RgbPixelTable = std::vector<boost::gil::rgb8_pixel_t>;
using Bitmap = boost::gil::rgb8_image_t;
using CountSet = boost::multi_array<Count, 2>;

// Cividis color gradation from yellow to blue
using ColorElement = uint8_t;
constexpr ColorElement LOW_COLOR_R = 0;
constexpr ColorElement LOW_COLOR_G = 32;
constexpr ColorElement LOW_COLOR_B = 81;
constexpr ColorElement HIGH_COLOR_R = 253;
constexpr ColorElement HIGH_COLOR_G = 233;
constexpr ColorElement HIGH_COLOR_B = 69;

struct ParamSet final {
    /// An x offset that is added in iterations
    Coordinate x_offset;
    /// A y offset that is added in iterations
    Coordinate y_offset;
    /// The maximum number of iterations
    Count max_iter;
    /// The width and height in pixels of an output image
    PixelSize n_pixels;
    /// A path to save counts as a table
    std::optional<std::filesystem::path> csv_filepath;
    /// A path to save counts as an image
    std::optional<std::filesystem::path> image_filepath;

    ParamSet(
        Coordinate arg_x_offset, Coordinate arg_y_offset,
        Count arg_max_iter, PixelSize arg_n_pixels,
        const std::optional<std::string>& arg_csv_filename,
        const std::optional<std::string>& arg_image_filename) :
        x_offset(arg_x_offset), y_offset(arg_y_offset),
        max_iter(arg_max_iter), n_pixels(arg_n_pixels) {
        // Can move constructive
        if (arg_csv_filename.has_value()) {
            csv_filepath = *arg_csv_filename;
        }
        if (arg_image_filename.has_value()) {
            image_filepath = *arg_image_filename;
        }
    }
};

inline auto norm2_sqr(const Point& point) {
    const auto real = point.real();
    const auto imag = point.imag();
    return real * real + imag * imag;
}

extern Point transform_point(const Point& from, const Point& offset);

extern Count converge_point(Coordinate point_x, Coordinate point_y,
                            const Point& point_offset, Count max_iter, Coordinate eps);

extern CountSet converge_point_set(const CoordinateSetView& xs, const CoordinateSetView& ys,
                                   const Point& point_offset, Count max_iter, Coordinate eps);

extern CoordinateSet map_coordinates(Coordinate half_length, PixelSize n_pixels);

extern CountSet scan_points(Coordinate x_offset, Coordinate y_offset, Count max_iter, PixelSize n_pixels);

extern RgbPixelTable make_gradient_colors(Count max_count);

extern Bitmap draw_image(const CountSet& count_set);

extern void draw(const ParamSet& params);

#endif // JULIASET_H
