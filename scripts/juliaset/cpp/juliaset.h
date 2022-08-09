#ifndef JULIASET_H_INCLUDED
#define JULIASET_H_INCLUDED

#include <boost/gil.hpp>
#include <boost/gil/extension/dynamic_image/dynamic_image_all.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/io/write_view.hpp>
#include <boost/multi_array.hpp>
#include <cmath>
#include <complex>
#if __cplusplus >= 202002L
#include <concepts>
#endif // C++20
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <thread>

/**
 Drawing Julia sets in C++
 */
namespace juliaset {
/// A count that represents how many times a point is transformed
using Count = int32_t;

/// A coordinate in screens
using Coordinate = float;

/// A number of pixels
using PixelSize = size_t;

/// A point as two coordinates in screens
using Point = std::complex<Coordinate>;

/// A set of coordinates and its view
using CoordinateSet = boost::multi_array<Coordinate, 1>;
using CoordinateSetView = CoordinateSet::const_array_view<1>::type;

/// An indexed RGB color table
using RgbPixelTable = std::vector<boost::gil::rgb8_pixel_t>;

/// RGB colors at pixels in a screen
using Bitmap = boost::gil::rgb8_image_t;

/// All counts in a screen
using CountSet = boost::multi_array<Count, 2>;

/// Red, green, or blue brightness
using ColorElement = uint8_t;

/// Default eps
inline constexpr auto DefaultEps = static_cast<Coordinate>(1e-5f);

// Cividis color gradation from blue to yellow
constexpr ColorElement LOW_COLOR_R = 0;
constexpr ColorElement LOW_COLOR_G = 32;
constexpr ColorElement LOW_COLOR_B = 81;
constexpr ColorElement HIGH_COLOR_R = 253;
constexpr ColorElement HIGH_COLOR_G = 233;
constexpr ColorElement HIGH_COLOR_B = 69;

/// A parameter set to draw
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

    /**
     * @param[in] arg_x_offset An x offset that is added in iterations
     * @param[in] arg_y_offset A y offset that is added in iterations
     * @param[in] arg_max_iter The maximum number of iterations
     * @param[in] arg_n_pixels The width and height in pixels of an output image
     * @param[in] arg_csv_filename A path to save counts as a table
     * @param[in] arg_image_filename A path to save counts as an image
     */
    ParamSet(Coordinate arg_x_offset, Coordinate arg_y_offset, Count arg_max_iter,
             PixelSize arg_n_pixels, const std::optional<std::string>& arg_csv_filename,
             const std::optional<std::string>& arg_image_filename)
        : x_offset(arg_x_offset), y_offset(arg_y_offset), max_iter(arg_max_iter),
          n_pixels(arg_n_pixels) {
        // Can move constructive
        if (arg_csv_filename.has_value()) {
            csv_filepath = arg_csv_filename.value();
        }
        if (arg_image_filename.has_value()) {
            image_filepath = arg_image_filename.value();
        }
    }
};

/**
 * @brief Returns the squared modulus of a complex number
 * @param[in] point A point
 * @return The squared modulus (2-norm) of the point
 */
inline auto norm2_sqr(const Point& point) {
    const auto real = point.real();
    const auto imag = point.imag();
    return real * real + imag * imag;
}

/**
 * @brief Returns a transformed point under the rule of Julia sets
 * @param[in] from An original point
 * @param[in] offset An offset to be added
 * @return A transformed point under the rule of Julia sets
 */
extern Point transform_point(const Point& from, const Point& offset);

/**
 * @brief Returns how many times a point is transformed
 * @param[in] point_x The x coordinate of a point
 * @param[in] point_y The y coordinate of a point
 * @param[in] point_offset An offset to be added to points
 * @param[in] max_iter The maximum number of iterations
 * @param[in] eps Tolerance to check if transformations are converged
 * @return How many times a point is transformed
 */
extern Count converge_point(Coordinate point_x, Coordinate point_y, const Point& point_offset,
                            Count max_iter, Coordinate eps);

/**
 * @brief Returns how many times each point in a screen is transformed
 * @param[in] xs A X-coordinates view of points in a screen
 * @param[in] ys A Y-coordinates view of points in a screen
 * @param[in] point_offset An offset to be added to points
 * @param[in] max_iter The maximum number of iterations
 * @param[in] eps Tolerance to check if transformations are converged
 * @return How many times each point in a screen is transformed
 */
extern CountSet converge_point_set(CoordinateSetView& xs, CoordinateSetView& ys,
                                   const Point& point_offset, Count max_iter, Coordinate eps);

/**
 * @brief Returns pixel coordinates on an axis in a screen
 * @param[in] half_length Maximum x and y coordinates relative to (0,0)
 * @param[in] n_pixels Numbers of pixels in X and Y axes
 * @return Pixel coordinates on an axis in a screen
 */
extern CoordinateSet map_coordinates(Coordinate half_length, PixelSize n_pixels);

/**
 * @brief Returns how many times each point in a screen is transformed
 * @param[in] x_offset An x offset that is added in iterations
 * @param[in] y_offset A y offset that is added in iterations
 * @param[in] max_iter The maximum number of iterations
 * @param[in] n_pixels Numbers of pixels in X and Y axes
 * @return How many times each point in a screen is transformed
 */
extern CountSet scan_points(Coordinate x_offset, Coordinate y_offset, Count max_iter,
                            PixelSize n_pixels);

/**
 * @brief Returns a gradient color table for [0..max_count]
 * @param[in] max_count The maximum index of the color table
 * @return A gradient color table for indexes [0..max_count]
 */
extern RgbPixelTable make_gradient_colors(Count max_count);

/**
 * @brief Draws a PNG image from an input screen
 * @param[in] count_set Counts of a Julia set in a screen
 * @return A PNG image from an input screen
 */
extern Bitmap draw_image(const CountSet& count_set);

/**
 * @brief Writes a count set table to a CSV file
 * @param[in] count_set Counts of a Julia set in a screen
 * @param[in] csv_filename A CSV filename to save the set
 */
extern void write_csv(const CountSet& count_set, const std::filesystem::path& csv_filename);

/**
 * @brief Draws a Julia set
 * @param[in] params A parameter set to draw
 */
extern void draw(const ParamSet& params);

/**
 * @brief Convert a number and check if convertible
 * @tparam[in] ToType The type of a target value
 * @tparam[in] FromType The type of a source value
 * @param[in] from A source value
 * @return The converted value
 */
template <typename ToType, typename FromType,
          std::enable_if_t<std::is_arithmetic_v<std::decay_t<FromType>>, std::nullptr_t> = nullptr>
ToType checked_cast(FromType&& from) {
#ifdef UNIT_TEST
    return boost::numeric_cast<ToType>(from);
#else
    return static_cast<ToType>(from);
#endif // UNIT_TEST
}

#if __cplusplus >= 202002L
/**
 * @brief Convert a number and check if convertible
 * @tparam[in] ToType The type of a target value
 * @tparam[in] FromType The type of a source value
 * @param[in] from A source value
 * @return The converted value
 */
template <typename ToType, typename FromType>
    requires std::integral<std::decay_t<FromType>> ||
    std::floating_point<std::decay_t<FromType>> ToType checked_cast_cpp20(FromType&& from) {
#ifdef UNIT_TEST
    return boost::numeric_cast<ToType>(from);
#else
    return static_cast<ToType>(from);
#endif // UNIT_TEST
}
#endif // C++20

} // namespace juliaset

#endif // JULIASET_H_INCLUDED
