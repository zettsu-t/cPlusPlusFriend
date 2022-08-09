#include "juliaset.h"

int main(int argc, char* argv[]) {
    using namespace juliaset;
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
        ("x_offset,x",
         boost::program_options::value<decltype(x_offset)>()->default_value(0.375),
         "X offset")
        ("y_offset,y",
         boost::program_options::value<decltype(y_offset)>()->default_value(0.375),
         "Y offset")
        ("max_iter,m",
         boost::program_options::value<decltype(max_iter)>()->default_value(100),
         "Max iterations")
        ("size,s",
         boost::program_options::value<decltype(n_pixels)>()->default_value(256),
         "Pixel size")
        ("csv,c",
         boost::program_options::value<decltype(csv_filename)>(),
         "CSV filename")
        ("image,o",
         boost::program_options::value<decltype(image_filename)>()->default_value("cpp_juliaset.png"),
         "PNG filename")
        ;

    boost::program_options::variables_map var_map;
    boost::program_options::store(parse_command_line(argc, argv, description), var_map);
    boost::program_options::notify(var_map);

    if (var_map.count("x_offset")) {
        x_offset = var_map["x_offset"].as<decltype(x_offset)>();
    }

    if (var_map.count("y_offset")) {
        y_offset = var_map["y_offset"].as<decltype(y_offset)>();
    }

    if (var_map.count("max_iter")) {
        max_iter = var_map["max_iter"].as<decltype(max_iter)>();
    }

    if (var_map.count("size")) {
        n_pixels = var_map["size"].as<decltype(n_pixels)>();
    }

    if (var_map.count("csv")) {
        csv_filename = var_map["csv"].as<decltype(csv_filename)>();
        csv_filepath = std::filesystem::path(csv_filename);
    }

    if (var_map.count("image")) {
        image_filename = var_map["image"].as<decltype(image_filename)>();
        image_filepath = std::filesystem::path(image_filename);
    }

    ParamSet params(x_offset, y_offset, max_iter, n_pixels, csv_filepath, image_filepath);
    draw(params);
    return 0;
}
