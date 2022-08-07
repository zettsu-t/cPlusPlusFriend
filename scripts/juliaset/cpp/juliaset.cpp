#include "juliaset.h"

int main(int argc, char* argv[]) {
    std::string cpp_filename("cpp.png");
    PixelSize n_pixels = 256;
    auto count_set = scan_points(0.375, 0.375, 75, n_pixels);
    auto cpp_img = draw_image(count_set);
    boost::gil::write_view(cpp_filename, view(cpp_img), boost::gil::png_tag());
    return 0;
}
