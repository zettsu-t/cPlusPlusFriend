#include <algorithm>
#include <iostream>
#include <random>
#include <tuple>
#include <vector>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

using Number = double;
using Numbers = std::vector<Number>;

void generateNumbers(Numbers& v) {
    std::random_device seed_gen;
    std::mt19937 engine(seed_gen());
    std::uniform_real_distribution<Number> dist(0.0, 1.0);
    std::for_each(v.begin(), v.end(), [&](auto& x) { x = dist(engine); });
}

std::tuple<Number, Number> accumulateNumbers(const Numbers& v) {
    using namespace boost::accumulators;
    accumulator_set<Number, stats<tag::mean, tag::variance>> acc;
    std::for_each(v.begin(), v.end(), [&](const auto& x) { acc(x); });
    return {mean(acc), variance(acc)};
}

int main(int argc, char* argv[]) {
    constexpr Numbers::size_type Size = 0x8000000ull;
    Numbers v(Size, 0.0);
    generateNumbers(v);
    const auto [mean, variance] = accumulateNumbers(v);
    std::cout << "mean:" << mean << " variance:" << variance << "\n";
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
