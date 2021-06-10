// This code finds the minimum integer N such that sum(1/i) for i=1..N >= given M
// This is based on the tweet below
// https://twitter.com/con_integral17/status/1398468512402665474
//
// Running
//   suminv 10
// finds the N for M = 10 and running without arguments
//   suminv
// executes built-in unit tests.

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
using Border = uint32_t;
using LessFlag = uint64_t;
using LoopIndex = size_t;

// *pInput has input parametes and stores values in all YMM registers.
LoopIndex callSumUpto(void* pInput, void* pOutput) {
    LoopIndex loopIndex = 0;
    asm volatile (
        "call sumUpto\n\t"
        :"=b"(loopIndex):"S"(pInput),"D"(pOutput):"memory");

    return loopIndex;
}

Number findMinIndex(LoopIndex divisorIndex, const LessFlag* lessFlagSet,
                    size_t elementsInRegister, const Number* divisorSet) {
    Number minDivisor = 0;

    for(auto i = decltype(elementsInRegister){0}; i < elementsInRegister; ++i) {
        if (lessFlagSet[i]) {
            const auto divisor = divisorSet[i + divisorIndex * elementsInRegister];
            if (minDivisor <= 0) {
                minDivisor = divisor;
            } else {
                minDivisor = std::min(divisor, minDivisor);
            }
        }
    }

    return minDivisor;
}

template<typename T, size_t N>
void printElements(std::ostream& os, const T (&es)[N]) {
    for(const auto& e : es) {
        os << e << " : ";
    }
}

template<typename T1, typename T2, typename T3, size_t N1, size_t N2, size_t N3>
void printResult(std::ostream& os, Number minDivisor, LoopIndex divisorIndex,
                 const T1 (&lessFlagSet)[N1], const T2 (&divisorSet)[N2], const T3 (&sumSet)[N3]) {
    os << "Answer: " << minDivisor << "\n";
    os << "Divisor set number: " << divisorIndex << "\n";

    os << "Less flags: ";
    printElements(os, lessFlagSet);
    os << "\n";

    os << "Divisors: ";
    printElements(os, divisorSet);
    os << "\n";

    os << "Sums: ";
    printElements(os, sumSet);
    os << "\n";
    return;
}

Number sumUpToBorder(Border border, bool quiet, std::ostream& os) {
    constexpr size_t ElementsInRegister = 4;  // A 256-bit YMM register holds four doubles or uint_64s;
    constexpr size_t SizeOfInputValues = 17;  // args + all YMM registers
    constexpr size_t SizeOfReturnValues = 7;  // Must larger than return values!
    constexpr size_t SizeOfFlags = 1;
    constexpr size_t SizeOfDivisors = 3;

    alignas(32) Border borderSet[ElementsInRegister] {0};
    alignas(32) Number paramSet[ElementsInRegister * SizeOfInputValues] {0};
    alignas(32) Number resultSet[ElementsInRegister * SizeOfReturnValues] {0};

    // 0 for border > value non-0 for otherwise
    alignas(32) LessFlag lessFlagSet[ElementsInRegister * SizeOfFlags] {0};
    alignas(32) Number   divisorSet[ElementsInRegister * SizeOfDivisors] {0};
    alignas(32) Number   sumSet[ElementsInRegister * SizeOfDivisors] {0};
    static_assert(sizeof(lessFlagSet) + sizeof(divisorSet) + sizeof(sumSet) <= sizeof(resultSet));

    std::fill(std::begin(borderSet), std::end(borderSet), border);
    std::memmove(paramSet, borderSet, sizeof(borderSet));

    const auto divisorIndex = callSumUpto(paramSet, resultSet);
    std::memmove(lessFlagSet, resultSet, sizeof(lessFlagSet));
    std::memmove(divisorSet, resultSet + ElementsInRegister * SizeOfFlags, sizeof(divisorSet));
    std::memmove(sumSet, resultSet + ElementsInRegister * (SizeOfFlags + SizeOfDivisors), sizeof(sumSet));

    const auto minDivisor = findMinIndex(divisorIndex, lessFlagSet, ElementsInRegister, divisorSet);
    os << std::setprecision(std::numeric_limits<double>::max_digits10);
    if (!quiet) {
        printResult(os, minDivisor, divisorIndex, lessFlagSet, divisorSet, sumSet);
    }
    return minDivisor;
}

// Assumes tolerance > 0
bool nearNumber(Number expected, Number actual, Number tolerance) {
    const auto diff = expected - actual;
    return ((-tolerance < diff) & (diff < tolerance));
}

bool nearNumber(Number expected, Number actual) {
    const Number Tolerance = 1e-7;
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
    Border border = 1;

    std::ostringstream os;
    for (const auto& expected : expectedSet) {
        const auto actual = sumUpToBorder(border, true, os);
        if (!nearNumber(expected, actual)) {
            std::cout << "A test failed in selfTestFixed()\n";
            failed |= true;
        }
        ++border;
    }

    return failed;
}

bool selfTestLoop(void) {
    constexpr Number maxIndex = 1e+9;
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
    Border border = 1;

    std::ostringstream os;
    for(const auto& expected : divisorSet) {
        const auto actual = sumUpToBorder(border, true, os);
        if (!nearNumber(expected, actual)) {
            std::cout << "A test failed in selfTestLoop()\n";
            failed |= true;
        }
        ++border;
    }

    return failed;
}

bool selfTestPrint(void) {
    bool failed = false;
    constexpr Border border = 10;
    const std::string expectedStr {"Answer: 12367"};
    constexpr Number expected = 12367;

    std::ostringstream os;
    auto actual = sumUpToBorder(border, false, os);
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
    int border = 0;
    if (argc > 1) {
        border = std::atoi(argv[1]);
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
    sumUpToBorder(border, false, std::cout);
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
