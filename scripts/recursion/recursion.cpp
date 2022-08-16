#include <cstdlib>
#include <iostream>
#include <random>

using RandGen = std::mt19937;

using RandNum = typename std::invoke_result<
    decltype(&RandGen::operator()), std::mt19937>::type;

inline RandNum make_randnum(RandGen& gen) {
    return gen() & 0xffffu;
}

inline RandNum accum(size_t count, RandGen& gen) {
    if (count == 0) {
        return 0;
    } else {
        const RandNum n = make_randnum(gen);
        std::cout << count << ":" << n << "\n";
        return n + accum(count - 1, gen);
    }
}

template <size_t count>
inline RandNum accum(RandGen& gen) {
    const RandNum n = make_randnum(gen);
    std::cout << count << ":" << n << "\n";
    return n + accum<count - 1>(gen);
}

template <>
inline RandNum accum<0>(RandGen& gen) {
    return 0;
}

int main(int argc, char* argv[]) {
    const int count = (argc > 1) ? std::atoi(argv[1]) : 4;
    std::random_device seed_gen;
    std::mt19937 gen(seed_gen());
    std::cout << accum(count, gen) << "\n\n";
    std::cout << accum<4>(gen) << "\n\n";
    return 0;
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
Last:
*/
