#include <bit>
#include <limits>

#ifdef UNIT_TEST_CPP
#include "code.h"
NumberVector cpp_popcount(NumberVectorArg xs)
#else
#include <Rcpp.h>
// [[Rcpp::export]]
Rcpp::NumericVector cpp_popcount(Rcpp::NumericVector xs)
#endif // UNIT_TEST_CPP
{
#ifndef UNIT_TEST_CPP
    using NumberVector = Rcpp::NumericVector;
#endif
    // Zero initialization
    NumberVector popcounts(xs.size());

    size_t i = 0;
    for (const auto& x : xs) {
        unsigned long uint_x = 0;
        if ((x >= 0) && (x <= std::numeric_limits<decltype(uint_x)>::max())) {
            uint_x = static_cast<decltype(uint_x)>(x);
        }
        auto y = std::popcount(uint_x);
        popcounts.at(i) = y;
        ++i;
    }
    return popcounts;
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
