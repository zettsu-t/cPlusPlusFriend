#include <cstdint>
#include <algorithm>
#include <fstream>
#include <random>
#include <string>
#include <unordered_map>

namespace {
    using ParamType = double;
    using ResultType = int;
    using RandEngine = std::mt19937;
    using SizeType = size_t;
    using SeedType = std::mt19937::result_type;

    struct ParamSet {
        ParamType size;
        ParamType prob;
        std::string filename;
    };

    const std::vector<ParamSet> AllParamSet {
        {1.0,  0.25, "nbinom_1.csv"},
        {1.0,  0.50, "nbinom_2.csv"},
        {1.0,  0.75, "nbinom_3.csv"},
        {2.0,  0.25, "nbinom_4.csv"},
        {3.0,  0.40, "nbinom_5.csv"},
        {5.0,  0.50, "nbinom_6.csv"},
        {10.0, 0.50, "nbinom_7.csv"},
        {15.0, 0.25, "nbinom_8.csv"},
        {15.0, 0.75, "nbinom_9.csv"},
    };
}

void rnbinomCpp(const auto& paramSet, SizeType n, SeedType seed) {
    const ParamType alpha = paramSet.size;
    // Notice that prob for R means 1 - prob in C++
    const ParamType beta = (1 - paramSet.prob) / paramSet.prob;
    std::mt19937 gen(seed);
    std::gamma_distribution<double> distGamma(alpha, beta);
    std::unordered_map<ResultType, SizeType> count;

    std::ofstream ofs(paramSet.filename);
    ofs << alpha << "\n" << beta << "\n";

    ResultType maxValue = 0;
    for (decltype(n) i=0; i<n; ++i) {
        std::poisson_distribution<ResultType> dist(distGamma(gen));
        auto value = dist(gen);
        count[value] += 1;
        maxValue = std::max(maxValue, value);
    }

    for (ResultType value=0; value<=maxValue; ++value) {
        ofs << count[value] << "\n";
    }
    return;
}

int main(int argc, char* argv[]) {
    constexpr size_t n = 1000000;
    constexpr SeedType seed = 123;
    for(const auto& paramSet : AllParamSet) {
        rnbinomCpp(paramSet, n, seed);
    }
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
