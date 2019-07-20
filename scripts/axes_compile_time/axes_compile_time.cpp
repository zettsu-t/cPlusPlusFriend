// Based on the Boost histogram example
// https://www.boost.org/doc/libs/1_70_0/libs/histogram/doc/html/histogram/getting_started.html
#include <algorithm>
#include <boost/format.hpp>
#include <boost/histogram.hpp>
#include <functional>
#include <iostream>
#include <sstream>
#include <vector>

namespace {
#include "axes_compile_time.inc"
}

int main(int argc, char* argv[]) {
    using namespace boost::histogram;

    auto h = make_histogram(axis::regular<>(48, -12.0, 12.0, "x"));
    std::for_each(data.begin(), data.end(), std::ref(h));

    std::ostringstream oss;
    for (auto x : indexed(h, coverage::all)) {
        oss << boost::format("bin %2i [%5.1f, %5.1f): %i\n") %
            x.index() % x.bin().lower() % x.bin().upper() % *x;
    }

    std::cout << oss.str() << std::flush;
    return 0;
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
