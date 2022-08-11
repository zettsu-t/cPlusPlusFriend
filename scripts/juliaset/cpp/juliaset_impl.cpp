#include "juliaset.h"
#include <algorithm>
#include <boost/algorithm/string/join.hpp>
#include <boost/cast.hpp>
#include <exception>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <type_traits>

namespace juliaset {
Point transform_point(const Point& from, const Point& offset) { return from * from + offset; }

Count converge_point(Coordinate point_x, Coordinate point_y, const Point& point_offset,
                     Count max_iter, Coordinate eps) {
    constexpr Coordinate limit_modulus = 4;
    Coordinate previous_modulus = limit_modulus * limit_modulus;
    Point z{point_x, point_y};

    Count count = 0;
    while (count < max_iter) {
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

CountSet converge_point_set(CoordinateSetView& xs, CoordinateSetView& ys, const Point& point_offset,
                            Count max_iter, Coordinate eps) {
    auto xs_size = xs.shape()[0];
    auto ys_size = ys.shape()[0];
    CountSet mat_counts(boost::extents[ys_size][xs_size]);

    decltype(ys_size) y_index{0};
    for (auto point_y = ys.begin(); point_y != ys.end(); ++point_y, ++y_index) {
        decltype(xs_size) x_index{0};
        for (auto point_x = xs.begin(); point_x != xs.end(); ++point_x, ++x_index) {
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
    const auto span = checked_cast<Coordinate>(n_pixels - 1);
    for (decltype(n_pixels) i{0}; i < n_pixels; ++i) {
        const auto coord = checked_cast<Coordinate>(i);
        const auto value = (coord * 2 * half_length / span) - half_length;
        coord_set[i] = value;
    }

    return coord_set;
}

CountSet scan_points(Coordinate x_offset, Coordinate y_offset, Count max_iter, PixelSize n_pixels) {
    const Coordinate half_length = std::sqrt(checked_cast<Coordinate>(2)) + 0.1f;
    const auto xs = map_coordinates(half_length, n_pixels);
    const auto ys = map_coordinates(half_length, n_pixels);
    const Point point_offset{x_offset, y_offset};
    constexpr auto eps = DefaultEps;

    auto n_xs = xs.shape()[0];
    auto n_ys = ys.shape()[0];
    CountSet mat_counts(boost::extents[n_ys][n_xs]);
    auto n_cpus = std::thread::hardware_concurrency();
    CoordinateSetView x_view = xs[boost::indices[decltype(xs)::index_range()]];

    using Index = decltype(n_ys);
    Index index_start = 0;
    Index index_span =
        checked_cast<Index>(std::ceil(checked_cast<double>(n_ys) / checked_cast<double>(n_cpus)));

    for (decltype(n_cpus) i{0}; i < n_cpus; ++i) {
        Index index_end = index_start + index_span;
        index_end = std::min(index_end, n_ys);
        CoordinateSetView y_view =
            ys[boost::indices[decltype(ys)::index_range(index_start, index_end)]];

        const auto sub_counts = converge_point_set(x_view, y_view, point_offset, max_iter, eps);
        mat_counts[boost::indices[decltype(mat_counts)::index_range(index_start, index_end)]
                                 [decltype(mat_counts)::index_range()]] = sub_counts;

        index_start = index_end;
    }

    return mat_counts;
}

RgbPixelTable make_gradient_colors(Count max_count) {
    RgbPixelTable table(std::max(0, max_count) + 1);

    if (max_count < 1) {
        table.at(0) = boost::gil::rgb8_pixel_t{HIGH_COLOR_R, HIGH_COLOR_G, HIGH_COLOR_B};
    } else {
        for (decltype(max_count) i{0}; i <= max_count; ++i) {
            auto inner_point = [i, max_count](ColorElement left, ColorElement right) {
                using ColorValue = double;
                const auto weight =
                    checked_cast<ColorValue>(i) / checked_cast<ColorValue>(max_count);
                auto value = checked_cast<ColorValue>(left) * (1.0 - weight) +
                             checked_cast<ColorValue>(right) * weight;

                value = std::clamp(
                    value, checked_cast<ColorValue>(std::numeric_limits<ColorElement>::min()),
                    checked_cast<ColorValue>(std::numeric_limits<ColorElement>::max()));
                return checked_cast<ColorElement>(value);
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
    for (decltype(n_ys) y{0}; y < n_ys; ++y) {
        auto x_view = count_set[boost::indices[y][CountSet::index_range()]];
        auto max_iter = std::max_element(x_view.begin(), x_view.end());
        if (max_iter != x_view.end()) {
            max_count = std::max(max_count, *max_iter);
        }
    }

    const auto color_table = make_gradient_colors(max_count);
    for (decltype(n_ys) y{0}; y < n_ys; ++y) {
        auto it = view(img).row_begin(y);
        for (decltype(n_xs) x{0}; x < n_xs; ++x, ++it) {
            *it = color_table.at(count_set[y][x]);
        }
    }

    return img;
}

[[nodiscard]] ExitStatus write_csv(const CountSet& count_set, const std::filesystem::path& csv_filename) {
    bool success = false;
    try {
        auto n_rows = count_set.shape()[0];
        std::ofstream os(csv_filename);

        for (decltype(n_rows) i_row{0}; i_row < n_rows; ++i_row) {
            const auto column = count_set[boost::indices[i_row][CountSet::index_range()]];
            std::vector<std::string> cells;
            std::transform(column.begin(), column.end(), std::back_inserter(cells),
                           [](auto i) { return std::to_string(i); });
            // We can alloc the 'joined' buffer once and replace trailing , to LF.
            std::string joined = boost::algorithm::join(cells, ",");
            os << joined << "\n";
        }

        os << std::flush;
        success = os.good();
        os.close();
        success &= os.good();
    } catch (std::exception& e) {
    }

    return (success) ? ExitStatus::SUCCESS : ExitStatus::FILE_ERROR;
}

[[nodiscard]] ExitStatus draw(const ParamSet& params) {
    const auto count_set =
        scan_points(params.x_offset, params.y_offset, params.max_iter, params.n_pixels);

    if (params.csv_filepath.has_value()) {
        const auto status = write_csv(count_set, params.csv_filepath.value());
        if (status != ExitStatus::SUCCESS) {
            return status;
        }
    }

    ExitStatus status = ExitStatus::SUCCESS;
    if (params.image_filepath.has_value()) {
        bool success = false;
        try {
            auto img = draw_image(count_set);
            const auto img_filename = params.image_filepath.value().string();
            boost::gil::write_view(img_filename, const_view(img), boost::gil::png_tag());
            success = true;
        } catch (std::exception& e) {
        }
        status = (success) ? ExitStatus::SUCCESS : ExitStatus::FILE_ERROR;
    }

    return status;
}

#define set_option_value(vmap, option_name, var_name) \
    do { \
        if (vmap.count(option_name.c_str())) { \
          var_name = vmap[option_name].as<decltype(var_name)>(); \
      } \
    } while(0) \

#define set_optional_path(vmap, option_name, var_name) \
    do { \
        if (vmap.count(option_name.c_str())) { \
            std::string filename = vmap[option_name].as<decltype(filename)>(); \
            var_name = decltype(var_name)::value_type(filename); \
        } \
    } while(0) \

ParamSet parse_args(int argc, const char* const argv[]) {
    const std::string long_opt_x_offset {"x_offset"};
    const std::string long_opt_y_offset {"y_offset"};
    const std::string long_opt_max_iter {"max_iter"};
    const std::string long_opt_size {"size"};
    const std::string long_opt_csv {"csv"};
    const std::string long_opt_image {"image"};

    const std::string opts_x_offset = long_opt_x_offset + ",x";
    const std::string opts_y_offset = long_opt_y_offset + ",y";
    const std::string opts_max_iter = long_opt_max_iter + ",m";
    const std::string opts_opt_size = long_opt_size + ",s";
    const std::string opts_csv = long_opt_csv + ",c";
    const std::string opts_image = long_opt_image + ",o";

    Coordinate x_offset {0};
    Coordinate y_offset {0};
    Count max_iter {0};
    PixelSize n_pixels {0};
    std::string csv_filename;
    std::string image_filename;
    std::optional<std::filesystem::path> csv_filepath;
    std::optional<std::filesystem::path> image_filepath;

    boost::program_options::options_description description("Options");
    description.add_options()
        (opts_x_offset.c_str(),
         boost::program_options::value<decltype(x_offset)>()->default_value(ParamSet::default_x_offset),
         "X offset")
        (opts_y_offset.c_str(),
         boost::program_options::value<decltype(y_offset)>()->default_value(ParamSet::default_y_offset),
         "Y offset")
        (opts_max_iter.c_str(),
         boost::program_options::value<decltype(max_iter)>()->default_value(ParamSet::default_max_iter),
         "Max iterations")
        (opts_opt_size.c_str(),
         boost::program_options::value<decltype(n_pixels)>()->default_value(ParamSet::default_n_pixels),
         "Pixel size")
        (opts_csv.c_str(),
         boost::program_options::value<decltype(csv_filename)>(),
         "CSV filename")
        (opts_image.c_str(),
         boost::program_options::value<decltype(image_filename)>()->default_value(ParamSet::default_image_filename),
         "PNG filename")
        ;

    boost::program_options::variables_map var_map;
    boost::program_options::store(parse_command_line(argc, argv, description), var_map);
    boost::program_options::notify(var_map);

    set_option_value(var_map, long_opt_x_offset, x_offset);
    set_option_value(var_map, long_opt_y_offset, y_offset);
    set_option_value(var_map, long_opt_max_iter, max_iter);
    set_option_value(var_map, long_opt_size, n_pixels);
    set_optional_path(var_map, long_opt_csv, csv_filepath);
    set_optional_path(var_map, long_opt_image, image_filepath);

    ParamSet params(x_offset, y_offset, max_iter, n_pixels, csv_filepath, image_filepath);
    return params;
}

} // namespace juliaset
