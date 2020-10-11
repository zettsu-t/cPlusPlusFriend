#include <cstdio>
#include <array>
#include <vector>

using LongInt = long long int;
constexpr LongInt fib(LongInt n) {
    if (n == 0) {
        return 0;
    } else if (n == 1) {
        return 1;
    }
    return fib(n-1) + fib(n-2);
}

template<LongInt n> constexpr LongInt fibt() {
    return fibt<n-1>() + fibt<n-2>();
}

template<> constexpr LongInt fibt<1>() {
    return 1;
}

template<> constexpr LongInt fibt<0>() {
    return 0;
}

template<size_t n> constexpr LongInt fibarray() {
    std::array<LongInt, n+1> v;
    for(decltype(n) i{0}; i<=n; ++i) {
        if (i == 0) {
            v.at(i) = 0;
        } else if (i == 1) {
            v.at(i) = i;
        } else if (i > 1) {
            v.at(i) = v.at(i-1) + v.at(i-2);
        }
    }
    return v.at(n);
}

template<size_t n> constexpr LongInt fibvector() {
    std::vector<LongInt> v(n+1, 0);
    for(decltype(n) i{0}; i<=n; ++i) {
        if (i == 0) {
            v.at(i) = 0;
        } else if (i == 1) {
            v.at(i) = i;
        } else if (i > 1) {
            v.at(i) = v.at(i-1) + v.at(i-2);
        }
    }
    return v.at(n);
}

// 0 1 1 2 3 5 8 13 21 34 55
int main(int argc, char* argv[]) {
#ifdef BASE
    std::printf("%lld\n", fib(9));
#else
#ifdef BAD_ARGUMENT
    std::printf("%lld\n", fib(BAD_ARGUMENT));
#else
    constexpr LongInt n = 90;  // fib(90) = 2880067194370816120
#ifdef NO_RETURN
    std::printf("%lld\n", fib(n));
#else
    std::printf("%lld\n", fibarray<static_cast<size_t>(n)>());
    std::printf("%lld\n", fibvector<static_cast<size_t>(n)>());
    std::printf("%lld\n", fibt<n>());
#endif
#endif
#endif
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
