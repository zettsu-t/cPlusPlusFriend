#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <random>
#include <tuple>
#include <vector>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <dSFMT.h>
#include <windows.h>

using Number = double;
using Numbers = std::vector<Number>;

class LapTime {
    using TimeIn100nsec = unsigned long long;
    using Time = double;
    static constexpr Time SecIn100nsec = 10000000;
public:
    LapTime(void) : startTime_(now()) {
        return;
    }

    virtual ~LapTime(void) = default;

    Time GetElapsedTime(void) {
        const auto endTime = now();
        const auto duration = endTime - startTime_;
        auto elapsedTime = static_cast<Time>(duration);
        elapsedTime /= SecIn100nsec;
        return elapsedTime;
    }

private:
    TimeIn100nsec now(void) {
        FILETIME timestamp;
        GetSystemTimeAsFileTime(&timestamp);

        TimeIn100nsec timeIn100nsec = timestamp.dwHighDateTime;
        for(size_t i=0; i<sizeof(timestamp.dwLowDateTime); ++i) {
            timeIn100nsec <<= 8;
        }
        timeIn100nsec += timestamp.dwLowDateTime;
        return timeIn100nsec;
    }

    TimeIn100nsec startTime_ {0};
};

void generateNumbers(Numbers& v) {
    LapTime laptime;

    std::random_device seed_gen;
    uint32_t seed = seed_gen();
    dsfmt_t dsfmt;
    dsfmt_init_gen_rand(&dsfmt, seed);

    // dsfmt_fill_array_open_close takes size as int
    const auto intSize = static_cast<int>(v.size());
    assert(static_cast<decltype(v.size())>(intSize) == v.size());
    dsfmt_fill_array_open_close(&dsfmt, v.data(), intSize);
    std::cout << "generateNumbers(dSFMT) " << laptime.GetElapsedTime() << "sec\n";
}

void generateNumbersStd(Numbers& v) {
    LapTime laptime;
    std::random_device seed_gen;
    std::mt19937 engine(seed_gen());
    std::uniform_real_distribution<Number> dist(0.0, 1.0);
    std::for_each(v.begin(), v.end(), [&](auto& x) { x = dist(engine); });
    std::cout << "generateNumbers(std) " << laptime.GetElapsedTime() << "sec\n";
}

std::tuple<Number, Number> accumulateNumbers(const Numbers& v, int loopSize) {
    LapTime laptime;
    using namespace boost::accumulators;
    accumulator_set<Number, stats<tag::mean, tag::variance>> acc;
    for(auto loopIndex = decltype(loopSize){0}; loopIndex < loopSize; ++loopIndex) {
        std::for_each(v.begin(), v.end(), [&](const auto& x) { acc(x); });
    }
    std::cout << "accumulateNumbers " << laptime.GetElapsedTime() << "sec\n";
    return {mean(acc), variance(acc)};
}

int main(int argc, char* argv[]) {
    bool setSize = (argc > 2);

    constexpr Numbers::size_type DefaultVecSize = 20000000ull;
    auto vecSize = DefaultVecSize;
    int loopSize = 1;

    if (setSize) {
        // vecSize must be shorter than INT_MAX
        vecSize = static_cast<decltype(vecSize)>(std::atoi(argv[1]));
        loopSize = std::atoi(argv[2]);
    }

    Numbers v(vecSize, 0.0);
    std::cout << "size: " << vecSize << " loop:" << loopSize << "\n";
    if (setSize) {
        generateNumbers(v);
    } else {
        generateNumbersStd(v);
    }

    const auto [mean, variance] = accumulateNumbers(v, loopSize);
    std::cout << "mean:" << mean << " variance:" << variance << "\n\n";
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
