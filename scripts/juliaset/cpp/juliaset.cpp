#include "juliaset.h"

int main(int argc, char* argv[]) {
    using namespace juliaset;
    const auto params = parse_args(argc, argv);
    return static_cast<int>(draw(params));
}
