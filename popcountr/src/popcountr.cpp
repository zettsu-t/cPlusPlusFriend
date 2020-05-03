#include <Rcpp.h>
using namespace Rcpp;

//' Fast population count
//'
//' A Rcpp implementation of population count using C++ compiler extensions and CPU instructions.
//'
//' @param x A single integer vector.
//' @return Population counts of each non-negative integer in x and
//'   an unspecified integer for NA_integer_. Results for negative integers
//'   are undefined.
// [[Rcpp::export]]
IntegerVector popcountr_(IntegerVector x) {
    static_assert(sizeof(decltype(x.at(0))) == sizeof(int), "Must be int");
    auto size = x.size();
    IntegerVector counts(size);

    for(decltype(size) i=0; i < size; ++i) {
        counts.at(i) = __builtin_popcount(x.at(i));
    };

    return counts;
}
