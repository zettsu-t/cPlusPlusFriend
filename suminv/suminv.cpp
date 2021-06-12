// This code finds the minimum integer N such that sum(1/i) for i=1..N >= given M
// This is based on the tweet below
// https://twitter.com/con_integral17/status/1398468512402665474
//
// Run
//   suminv 10
// and it finds the N for M = 10.
//
// Run without arguments
//   suminv
// and it executes built-in unit tests.
//
// suminv 10 4
// uses a 4-stage pipelining. 1 to 'MaxSizeOfDivisors' pipelining is available.
//
// suminv 10 0
// does not use SIMD parallel divisions.

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using Number = double;
using NumberSet = std::vector<Number>;
using Border = uint32_t;
using SizeOfUnits = size_t;
constexpr SizeOfUnits MaxSizeOfDivisors = 8;
constexpr SizeOfUnits SizeOfUnitsTested = MaxSizeOfDivisors + 1;

// *pInput has input parameters and stores values in all YMM registers.
void callSumUpto(void* pInput, void* pOutput, SizeOfUnits sizeOfDivisors, bool noParallel) {
    uint64_t arg = sizeOfDivisors;
    if (noParallel) {
        arg = 0;
    }

    asm volatile (
        "call sumUpto\n\t"
        ::"S"(pInput),"D"(pOutput),"a"(arg):"memory");

    return;
}

Number findMinDivisor(Border border, const NumberSet& divisorSet, const NumberSet& sumSet) {
    Number lowerBound = border;
    Number minDivisor = 0;

    const auto n = divisorSet.size();
    if (n != sumSet.size()) {
        return 0;
    }

    for(auto i = decltype(n){0}; i < n; ++i) {
        if (sumSet.at(i) >= lowerBound) {
            const auto divisor = divisorSet.at(i);
            if (minDivisor <= 0) {
                minDivisor = divisor;
            } else {
                minDivisor = std::min(divisor, minDivisor);
            }
        }
    }

    return minDivisor;
}

template<typename T>
void printElements(std::ostream& os, const T& es) {
    for(const auto& e : es) {
        os << e << " : ";
    }
}

void printResult(std::ostream& os, Number minDivisor,
                 const NumberSet& divisorSet, const NumberSet& sumSet) {
    os << "Answer: " << minDivisor << "\n";

    os << "Divisors: ";
    printElements(os, divisorSet);
    os << "\n";

    os << "Sums: ";
    printElements(os, sumSet);
    os << "\n";
    return;
}

Number sumUpToBorder(Border border, SizeOfUnits sizeOfUnits, bool quiet, std::ostream& os) {
    constexpr size_t ElementsInRegister = 4;  // A 256-bit YMM register holds four doubles or uint_64s
    constexpr size_t SizeOfInputValues = 17;  // Parameters and all YMM registers
    constexpr size_t SizeOfReturnValues = MaxSizeOfDivisors * 2;  // Must larger than return values!

    alignas(32) Border borderSet[ElementsInRegister] {0};
    alignas(32) Number paramSet[ElementsInRegister * SizeOfInputValues] {0};
    alignas(32) Number resultSet[ElementsInRegister * SizeOfReturnValues] {0};

    std::fill(std::begin(borderSet), std::end(borderSet), border);
    std::memmove(paramSet, borderSet, sizeof(borderSet));
    static_assert(sizeof(paramSet) >= sizeof(borderSet), "Too small");

    // 0 for border >= value non-0 for otherwise
    const bool NoParallel = (sizeOfUnits == 0);

    // NoParallel = true
    //   sum 1/i sequential
    // NoParallel = false
    //   sum sum 1/[i..(i + 4 * sizeOfUnits - 1)] parallel
    SizeOfUnits sizeOfDivisors = std::max(decltype(sizeOfUnits){1}, sizeOfUnits);
    sizeOfDivisors = std::min(sizeOfDivisors, MaxSizeOfDivisors);
    SizeOfUnits sizeOfElements = sizeOfDivisors * ElementsInRegister;
    NumberSet divisorSet(sizeOfElements, 0);
    NumberSet sumSet(sizeOfElements, 0);

    callSumUpto(paramSet, resultSet, sizeOfDivisors, NoParallel);
    for(auto i = decltype(sizeOfElements){0}; i < sizeOfElements; ++i) {
        divisorSet.at(i) = resultSet[i];
        sumSet.at(i) = resultSet[i + ElementsInRegister * MaxSizeOfDivisors];
    }

    const auto minDivisor = findMinDivisor(border, divisorSet, sumSet);
    os << std::setprecision(std::numeric_limits<double>::max_digits10);
    if (!quiet) {
        printResult(os, minDivisor, divisorSet, sumSet);
    }
    return minDivisor;
}

bool nearNumber(Number expected, Number actual, Number tolerance) {
    const auto diff = expected - actual;
    const Number tol = std::fabs(tolerance);
    return ((-tol < diff) & (diff < tol));
}

bool nearNumber(Number expected, Number actual) {
    constexpr Number Tolerance = 1e-7;
    return nearNumber(expected, actual, Tolerance);
}

bool sameInt(Number a, Number b) {
    constexpr Number Tolerance = 0.5;
    return nearNumber(std::floor(a), std::floor(b), Tolerance);
}

bool selfTestCompare(void) {
    bool failed = false;
    const bool actual = nearNumber(1.0, 1.0) &
        !nearNumber(1.0, 1.1) &
        !nearNumber(1.0, 0.9) &
        sameInt(1.0, 1.0) &
        sameInt(1.9, 1.0) &
        sameInt(1.0, 1.9) &
        !sameInt(1.0, 2.0) &
        !sameInt(2.0, 1.0);

    if (!actual) {
        std::cout << "A test failed in selfTestCompare()\n";
        failed |= true;
    }

    return failed;
}

bool selfTestFixed(void) {
    constexpr Number expectedSet[] {
        1, 4, 11, 31, 83,
        227, 616, 1674, 4550, 12367,
        33617, 91380, 248397, 675214, 1835421,
        4989191, 13562027, 36865412
    };

    bool failed = false;

    for(SizeOfUnits sizeOfUnits = 0; sizeOfUnits < SizeOfUnitsTested; ++sizeOfUnits) {
        Border border = 1;
        std::ostringstream os;
        for (const auto& expected : expectedSet) {
            const auto actual = sumUpToBorder(border, sizeOfUnits, true, os);
            if (!nearNumber(expected, actual)) {
                std::cout << "A test failed in selfTestFixed()\n";
                failed |= true;
            }
            ++border;
        }
    }

    return failed;
}

bool selfTestLoop(void) {
    constexpr Number maxIndex = 1e+8;
    Number sum = 0;
    std::vector<Number> divisorSet;

    for (Number index = 1; index <= maxIndex; index += 1.0) {
        const auto previousSum = sum;
        sum += 1.0 / index;
        if (!sameInt(sum, previousSum)) {
            divisorSet.push_back(index);
        }
    }

    if (divisorSet.empty()) {
        return true;
    }

    bool failed = false;

    for(SizeOfUnits sizeOfUnits = 0; sizeOfUnits < SizeOfUnitsTested; ++sizeOfUnits) {
        Border border = 1;
        std::ostringstream os;
        for(const auto& expected : divisorSet) {
            const auto actual = sumUpToBorder(border, sizeOfUnits, true, os);
            if (!nearNumber(expected, actual)) {
                std::cout << "A test failed in selfTestLoop()\n";
                failed |= true;
            }
            ++border;
        }
    }

    return failed;
}

bool selfTestPrint(void) {
    bool failed = false;
    constexpr Border border = 10;
    const std::string expectedStr {"Answer: 12367"};
    constexpr Number expected = 12367;

    std::ostringstream os;
    auto actual = sumUpToBorder(border, 0, false, os);
    if (!nearNumber(expected, actual)) {
        std::cout << "A test failed in selfTestPrint()\n";
        failed |= true;
    }

    const std::string actualStr = os.str();
    if (actualStr.find(expectedStr) == std::string::npos) {
        std::cout << "A test failed in selfTestPrint()\n";
        failed |= true;
    }

    return failed;
}

bool selfTest(void) {
    return selfTestCompare() | selfTestFixed() | selfTestLoop() | selfTestPrint();
}

int main(int argc, char* argv[]) {
    Border border = 0;
    SizeOfUnits sizeOfUnits = 0;

    if (argc > 1) {
        std::istringstream is(argv[1]);
        is >> border;
    }

    if (argc > 2) {
        std::istringstream is(argv[2]);
        is >> sizeOfUnits;
    }

    // No arguments
    if (border == 0) {
        if (selfTest()) {
            return 1;
        } else {
            std::cout << "All tests passed!\n";
            return 0;
        }
    }

    // With an argument
    sumUpToBorder(border, sizeOfUnits, false, std::cout);
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
