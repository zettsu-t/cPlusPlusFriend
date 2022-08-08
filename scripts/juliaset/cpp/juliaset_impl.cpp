#include "juliaset.h"
#include <boost/algorithm/string/join.hpp>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>

Point transform_point(const Point& from, const Point& offset) {
    return from * from + offset;
}

Count converge_point(Coordinate point_x, Coordinate point_y,
                     const Point& point_offset, Count max_iter, Coordinate eps) {
    constexpr Coordinate limit_modulus = 4.0;
    Coordinate previous_modulus = limit_modulus * limit_modulus;
    Point z {point_x, point_y};

    Count count = 0;
    while(count < max_iter) {
        z = transform_point(z, point_offset);
        const auto z_modulus = norm2_sqr(z);
        if (z_modulus > limit_modulus) {
            break;
        }
        if (std::fabs(previous_modulus - z_modulus) < eps) {
            break;
        }
        count += 1;
        previous_modulus = z_modulus;
    }

    return count;
}

CountSet converge_point_set(const CoordinateSetView& xs, const CoordinateSetView& ys,
                            const Point& point_offset, Count max_iter, Coordinate eps) {
    auto xs_size = xs.shape()[0];
    auto ys_size = ys.shape()[0];
    CountSet mat_counts(boost::extents[ys_size][xs_size]);

    decltype(ys_size) y_index {0};
    for(auto point_y = ys.begin(); point_y != ys.end(); ++point_y, ++y_index) {
        decltype(xs_size) x_index {0};
        for(auto point_x = xs.begin(); point_x != xs.end(); ++point_x, ++x_index) {
            mat_counts[y_index][x_index] =
                converge_point(*point_x, *point_y, point_offset, max_iter, eps);
        }
    }

    return mat_counts;
}

CoordinateSet map_coordinates(Coordinate half_length, PixelSize n_pixels) {
    if (n_pixels == 1) {
        return CoordinateSet(boost::extents[1]);
    } else if (n_pixels < 1) {
        return CoordinateSet(boost::extents[0]);
    }

    auto coord_set = CoordinateSet(boost::extents[n_pixels]);
    const auto span = static_cast<Coordinate>(n_pixels - 1);
    for(decltype(n_pixels) i {0}; i < n_pixels; ++i) {
        const auto coord = static_cast<Coordinate>(i);
        const auto value = (coord * 2 * half_length / span) - half_length;
        coord_set[i] = value;
    }

    return coord_set;
}

CountSet scan_points(Coordinate x_offset, Coordinate y_offset, Count max_iter, PixelSize n_pixels) {
    constexpr Coordinate half_length = std::sqrt(static_cast<Coordinate>(2)) + 0.1f;
    auto xs = map_coordinates(half_length, n_pixels);
    auto ys = map_coordinates(half_length, n_pixels);
    const Point point_offset {x_offset, y_offset};
    constexpr auto eps = DefaultEps;

    auto n_xs = xs.shape()[0];
    auto n_ys = ys.shape()[0];
    CountSet mat_counts(boost::extents[n_ys][n_xs]);
    auto n_cpus = std::thread::hardware_concurrency();
    auto x_view = xs[boost::indices[decltype(ys)::index_range(0, n_xs)]];

    using Index = decltype(n_ys);
    Index index_start = 0;
    Index index_span = static_cast<Index>(std::ceil(static_cast<double>(n_ys) / static_cast<double>(n_cpus)));

    for(decltype(n_cpus) i {0}; i < n_cpus; ++i) {
        Index index_end = index_start + index_span;
        index_end = std::min(index_end, n_ys);
        decltype(ys)::array_view<1>::type y_view =
            ys[boost::indices[decltype(ys)::index_range(index_start, index_end)]];

        const auto sub_counts = converge_point_set(x_view, y_view, point_offset, max_iter, eps);
        mat_counts[
            boost::indices
            [decltype(mat_counts)::index_range(index_start, index_end)]
            [decltype(mat_counts)::index_range(0, n_xs)]
            ] = sub_counts;

        index_start = index_end;
    }

    return mat_counts;
}

RgbPixelTable make_gradient_colors(Count max_count) {
    RgbPixelTable table(max_count + 1);

    if (max_count < 1) {
        table.at(0) = boost::gil::rgb8_pixel_t{HIGH_COLOR_R, HIGH_COLOR_G, HIGH_COLOR_B};
    } else {
        for(decltype(max_count) i {0}; i <= max_count; ++i) {
            auto inner_point = [i, max_count](ColorElement left, ColorElement right) {
                using ColorValue = double;
                const auto weight = static_cast<ColorValue>(i) / static_cast<ColorValue>(max_count);
                auto value = static_cast<ColorValue>(left) * (1.0 - weight) +
                    static_cast<ColorValue>(right) * weight;

                value = std::clamp(
                    value,
                    static_cast<ColorValue>(std::numeric_limits<ColorElement>::min()),
                    static_cast<ColorValue>(std::numeric_limits<ColorElement>::max()));
                return static_cast<ColorElement>(value);
            };

            const auto r = inner_point(LOW_COLOR_R, HIGH_COLOR_R);
            const auto g = inner_point(LOW_COLOR_G, HIGH_COLOR_G);
            const auto b = inner_point(LOW_COLOR_B, HIGH_COLOR_B);
            table.at(i) = boost::gil::rgb8_pixel_t{r, g, b};
        }
    }

    return table;
}

Bitmap draw_image(const CountSet& count_set) {
    auto n_ys = count_set.shape()[0];
    auto n_xs = count_set.shape()[1];
    Bitmap img(n_xs, n_ys);

    Count max_count = 0;
    for(decltype(n_ys) y {0}; y < n_ys; ++y) {
        auto x_view = count_set[boost::indices[y][CountSet::index_range(0, n_xs)]];
        auto max_iter = std::max_element(x_view.begin(), x_view.end());
        if (max_iter != x_view.end()) {
            max_count = std::max(max_count, *max_iter);
        }
    }

    const auto color_table = make_gradient_colors(max_count);
    for(decltype(n_ys) y {0}; y < n_ys; ++y) {
        auto it = view(img).row_begin(y);
        for(decltype(n_xs) x {0}; x < n_xs; ++x, ++it) {
            *it = color_table.at(count_set[y][x]);
        }
    }

    return img;
}

void write_csv(const CountSet& count_set, const std::filesystem::path& csv_filename) {
    auto n_rows = count_set.shape()[0];
    std::ofstream os(csv_filename);

    for(decltype(n_rows) i_row {0}; i_row < n_rows; ++i_row) {
        const auto column = count_set[boost::indices[i_row][CountSet::index_range()]];
        std::vector<std::string> cells;
        std::transform(column.begin(), column.end(), std::back_inserter(cells),
                       [](auto i) { return std::to_string(i); });
        std::string joined = boost::algorithm::join(cells, ",");
        os << joined << "\n";
    }
    os << std::flush;
}

void draw(const ParamSet& params) {
    const auto count_set = scan_points(params.x_offset, params.y_offset,
        params.max_iter, params.n_pixels);

    if (params.csv_filepath.has_value()) {
        write_csv(count_set, params.csv_filepath.value());
    }

    if (params.image_filepath.has_value()) {
        auto img = draw_image(count_set);
        const auto& img_filename = params.image_filepath.value();
        boost::gil::write_view(img_filename.string(), view(img), boost::gil::png_tag());
    }

    return;
}
