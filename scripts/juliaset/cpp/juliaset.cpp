#include "juliaset.h"

int main(int argc, char* argv[]) {
    const std::optional<std::string> csv_filename{"cpp.csv"};
    const std::optional<std::string> image_filename{"cpp.png"};
    const ParamSet params(0.375, 0.375, 75, 256, csv_filename, image_filename);
    draw(params);
    return 0;
}
