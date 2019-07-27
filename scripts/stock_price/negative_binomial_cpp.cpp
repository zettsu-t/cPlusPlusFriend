#include <cstdint>
#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>

#include <boost/math/distributions/gamma.hpp>
#include <boost/math/distributions/negative_binomial.hpp>

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

    // Cannot use alpha as integers
    std::negative_binomial_distribution<int> distNB(alpha, beta);

    // Makes random values
    ResultType maxValue = 0;
    for (decltype(n) i=0; i<n; ++i) {
        std::poisson_distribution<ResultType> dist(distGamma(gen));
        const auto value = dist(gen);
        count[value] += 1;
        maxValue = std::max(maxValue, value);
    }

    // Can eliminate outliers
    const SizeType sizeLimit = n + 100; //  n / 100;
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

template <typename CdfGenerator>
std::vector<DensityType> GenerateCdf(ResultType count, CdfGenerator& cdfGenerator) {
    std::vector<DensityType> vec;
    for(ResultType i=0; i<count; ++i) {
        vec.push_back(cdfGenerator(i));
    }
    return vec;
}

template <typename Vec>
void PrintVec(const Vec& vec, std::ostream& os) {
    const auto size = vec.size();
    for(auto i = decltype(size){0}; i<size; ++i) {
        os << vec.at(i);
        if ((i + 1) < size) {
            os << ",";
        }
    }
    os << "\n";
    return;
}

void nbinomCppCdf(void) {
    constexpr DensityType nbSize = 2.0;
    constexpr DensityType nbProb = 0.25;

    boost::math::negative_binomial distNB(nbSize, nbProb);
    auto funcNB = [&distNB](ResultType i) {
        return boost::math::cdf(distNB, i);
    };

    auto cdfNB = std::bind(funcNB, std::placeholders::_1);
    constexpr ResultType count = 10;
    std::ostringstream osNB;
    const auto actualNB = GenerateCdf(count, cdfNB);
    PrintVec(actualNB, osNB);

    // R pnbinom(seq(0, 9, 1), size=2.0, prob=0.25)
    // [1] 0.0625000 0.1562500 0.2617188 0.3671875 0.4660645 0.5550537 0.6329193 0.6996613
    // [9] 0.7559748 0.8029027
    std::cout << osNB.str();

    constexpr DensityType shape = 1.0;
    constexpr DensityType scale = 2.0;

    boost::math::gamma_distribution<double> distGamma(shape, scale);
    auto funcGamma = [&distGamma](ResultType i) {
        return boost::math::cdf(distGamma, i);
    };

    auto cdfGamma = std::bind(funcGamma, std::placeholders::_1);
    const auto actualGamma = GenerateCdf(count, cdfGamma);

    // R pgamma(seq(0, 9, 1), shape=1.0, scale=2.0)
    // [1] 0.0000000 0.3934693 0.6321206 0.7768698 0.8646647 0.9179150 0.9502129 0.9698026
    // [9] 0.9816844 0.9888910
    std::ostringstream osGamma;
    PrintVec(actualGamma, osGamma);
    std::cout << osGamma.str() << std::endl;
}

int main(int argc, char* argv[]) {
    rnbinomCppAll();
    nbinomCppCdf();
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
