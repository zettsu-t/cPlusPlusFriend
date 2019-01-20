// [[Rcpp::depends(BH)]]
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <type_traits>

#ifndef TESTING_FROM_MAIN
#include <Rcpp.h>
// 本当はsize_t(と同等のRcpp型)のベクタでなければならない
using IndexVector = Rcpp::NumericVector;
#else
namespace Rcpp {
    using NumericVector = std::vector<double>;
    using CharacterVector = std::vector<std::string>;
}
using IndexVector = std::vector<size_t>;
#endif

#if __cplusplus > 201402L
using ContainerIndex = std::invoke_result_t<decltype(&Rcpp::NumericVector::size), Rcpp::NumericVector>;
#else
using ContainerIndex = std::result_of_t<decltype(&Rcpp::NumericVector::size)(Rcpp::NumericVector)>;
#endif

template <typename T>
IndexVector find_lower_bound(const T& vec, const T& keys) {
    IndexVector result;
    for(const auto& key : keys) {
        ContainerIndex pos = 0;
        auto it = std::lower_bound(vec.begin(), vec.end(), key);
        if (it != vec.end()) {
            pos = std::distance(vec.begin(), it) + 1;
        }
        result.push_back(pos);
    }

    // Returns the 1-based index if the value is found in the vec_number, 0 otherwise
    return result;
}

#ifndef TESTING_FROM_MAIN
// [[Rcpp::export]]
IndexVector lower_bound_number(Rcpp::NumericVector vec, Rcpp::NumericVector value) {
    return find_lower_bound(vec, value);
}

// [[Rcpp::export]]
IndexVector lower_bound_string(Rcpp::CharacterVector vec, Rcpp::CharacterVector value) {
    return find_lower_bound(vec, value);
}

#else
IndexVector lower_bound_number(const Rcpp::NumericVector& vec, const Rcpp::NumericVector& value) {
    return find_lower_bound(vec, value);
}

IndexVector lower_bound_string(const Rcpp::CharacterVector& vec, const Rcpp::CharacterVector& value) {
    return find_lower_bound(vec, value);
}
#endif

#ifdef TESTING_FROM_MAIN
template <typename T, typename F>
int test_all(const T& vec, const T& keys, F& tested_func) {
    auto vectorSize = keys.size();
    IndexVector expected;
    for(decltype(vectorSize) i = 0; i < vectorSize; ++i) {
        expected.push_back((i + 1) % vectorSize);
    }

    const auto actual = tested_func(vec, keys);
    if ((actual.size() != vectorSize) || (vectorSize == 0)) {
        return 1;
    }

    for(decltype(vectorSize) i = 0; i < vectorSize; ++i) {
        std::cout << actual.at(i);
        if (actual.at(i) != expected.at(i)) {
            return 1;
        }
    }

    std::cout << "\n";
    return 0;
}

int main(int argc, char* argv[]) {
    int result = 0;
    const Rcpp::NumericVector vec_number {1.0, 2.0, 3.0, 4.0};
    const Rcpp::NumericVector keys_number {0.5, 1.5, 2.5, 3.5, 4.5};
    result |= test_all(vec_number, keys_number, lower_bound_number);

    const Rcpp::CharacterVector vec_string {"B", "BD", "BDF", "D"};
    const Rcpp::CharacterVector keys_string {"A", "BC", "BDE", "C", "E"};
    result |= test_all(vec_string, keys_string, lower_bound_string);
    return result;
}
#endif

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
