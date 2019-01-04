#include <cstdint>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <random>
#include <string>
#include <unordered_map>

namespace {
    using ParamType = double;
    using ResultType = int;
    using DensityType = double;
    using RandEngine = std::mt19937;
    using SizeType = size_t;
    using SeedType = std::mt19937::result_type;

    struct ParamSet {
        ParamType size;
        ParamType prob;
    };

    struct FileSet {
        SizeType n;
        std::string filename;
    };

    const std::vector<ParamType> SizeSet {2, 4, 6, 8};
    const std::vector<ParamType> ProbSet {0.1, 0.15, 0.25, 0.5};
    const std::vector<FileSet> AllFileSet {
        {100, "nbinom100.csv"},
        {1000, "nbinom1k.csv"},
        {10000, "nbinom10k.csv"},
        {100000, "nbinom100k.csv"},
    };
}

void rnbinomCpp(const auto& paramSet, SizeType n, SeedType seed, std::ofstream& ofs) {
    const ParamType alpha = paramSet.size;
    // Notice that prob for R means 1 - prob in C++
    const ParamType prob = paramSet.prob;
    const ParamType beta = (1 - prob) / prob;
    std::mt19937 gen(seed);
    std::gamma_distribution<double> distGamma(alpha, beta);
    std::unordered_map<ResultType, SizeType> count;

    // Makes random values
    ResultType maxValue = 0;
    for (decltype(n) i=0; i<n; ++i) {
        std::poisson_distribution<ResultType> dist(distGamma(gen));
        const auto value = dist(gen);
        count[value] += 1;
        maxValue = std::max(maxValue, value);
    }

    // Can eliminate outliers
    const SizeType sizeLimit = n - n / 100;
    SizeType totalCount = 0;
    for (ResultType value=0; value<=maxValue; ++value) {
        // Writes densities in a long format
        const auto binCount = count[value];
        totalCount += binCount;
        const DensityType density = static_cast<DensityType>(binCount) / static_cast<DensityType>(n);
        ofs << alpha << "," << beta << "," << prob << "," << value << "," << density << "\n";
        if (totalCount >= sizeLimit) {
            break;
        }
    }
    return;
}

void rnbinomCppAll(void) {
    constexpr SeedType seed = 123;

    for(const auto& fileSet :AllFileSet) {
        // Writes titles
        std::ofstream ofs(fileSet.filename);
        ofs << "alpha,beta,prob,x,density" << std::endl;

        for(const auto& size : SizeSet) {
            for(const auto& prob : ProbSet) {
                const ParamSet paramSet {size, prob};
                rnbinomCpp(paramSet, fileSet.n, seed, ofs);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    rnbinomCppAll();
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
