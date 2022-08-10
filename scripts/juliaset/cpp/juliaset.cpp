#include "juliaset.h"

int main(int argc, char* argv[]) {
    using namespace juliaset;
    const auto params = parse_args(argc, argv);
    draw(params);
    return 0;
}
