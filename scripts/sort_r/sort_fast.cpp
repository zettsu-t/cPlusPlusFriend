// [[Rcpp::depends(BH)]]
#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <type_traits>
#include <utility>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/iterator_range.hpp>

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

using IndexVectorInner = std::vector<size_t>;

#if __cplusplus > 201402L
using ContainerIndex = std::invoke_result_t<decltype(&Rcpp::NumericVector::size), Rcpp::NumericVector>;
#else
using ContainerIndex = std::result_of_t<decltype(&Rcpp::NumericVector::size)(Rcpp::NumericVector)>;
#endif

template <template<class> class Compare, typename T, typename U>
IndexVectorInner find_lower_bound(const T& vec, const T& keys, U&& iBegin, U&& iEnd) {
    IndexVectorInner results;
    for(const auto& key : keys) {
        // Resolve types which proxies returns
        auto element = *iBegin;
        auto it = std::lower_bound(iBegin, iEnd, key, Compare<decltype(element)>());
        ContainerIndex pos = 0;
        if (it != iEnd) {
            pos = std::distance(iBegin, it) + 1;
        }
        results.push_back(pos);
    }

    // Returns the 1-based index if the value is found in the vec_number, 0 otherwise
    return results;
}

// The vec must be sorted in an ascending order
template <typename T>
IndexVector find_ceil(const T& vec, const T& keys) {
    IndexVector results;
    const auto rawResults = find_lower_bound<std::less>(vec, keys, vec.begin(), vec.end());
    for (const auto& result : rawResults) {
        results.push_back(static_cast<decltype(rawResults[0])>(result));
    }
    return results;
}

// Rcpp may causes clashes
// The vec must be sorted in an ascending order
template <typename T>
IndexVector find_floor(const T& vec, const T& keys) {
    const IndexVectorInner::size_type topIndex = vec.size() + 1;
#if __GNUC__ > 6
    const auto rawResults = find_lower_bound<std::greater>(
        vec, keys, vec.rbegin(), vec.rend());
#else
    const auto rawResults = find_lower_bound<std::greater>(
        vec, keys,
        boost::make_reverse_iterator(vec.end()),
        boost::make_reverse_iterator(vec.begin()));
#endif
    IndexVector results;
    for (const auto& result : rawResults) {
        results.push_back(static_cast<decltype(rawResults[0])>(result ? (topIndex - result) : result));
    }
    return results;
}

// Workaround for Rcpp
// The vec must be sorted in a descending order
template <typename T>
IndexVector find_floor_reverse(const T& vec, const T& keys) {
    IndexVector results;
    const auto rawResults = find_lower_bound<std::greater>(vec, keys, vec.begin(), vec.end());
    for (const auto& result : rawResults) {
        results.push_back(static_cast<decltype(rawResults[0])>(result));
    }
    return results;
}

#ifndef TESTING_FROM_MAIN
// [[Rcpp::export]]
IndexVector find_ceil_number(Rcpp::NumericVector vec, Rcpp::NumericVector value) {
    return find_ceil(vec, value);
}

// [[Rcpp::export]]
IndexVector find_ceil_string(Rcpp::CharacterVector vec, Rcpp::CharacterVector value) {
    return find_ceil(vec, value);
}

// [[Rcpp::export]]
IndexVector find_floor_number(Rcpp::NumericVector vec, Rcpp::NumericVector value) {
    return find_floor(vec, value);
}

// [[Rcpp::export]]
IndexVector find_floor_string(Rcpp::CharacterVector vec, Rcpp::CharacterVector value) {
    return find_floor(vec, value);
}

// [[Rcpp::export]]
IndexVector find_floor_reverse_string(Rcpp::CharacterVector vec, Rcpp::CharacterVector value) {
    return find_floor_reverse(vec, value);
}

// [[Rcpp::export]]
Rcpp::CharacterVector sort_string_set(Rcpp::CharacterVector vec) {
    std::vector<std::string> string_set;
    for(const auto& v : vec) {
        auto s = Rcpp::as<std::string>(v);
        string_set.push_back(s);
    }
    std::sort(string_set.begin(), string_set.end());

    Rcpp::CharacterVector result;
    for(const auto& v : string_set) {
        result.push_back(v);
    }
    return result;
}

#else
IndexVector find_ceil_number(const Rcpp::NumericVector& vec, const Rcpp::NumericVector& value) {
    return find_ceil(vec, value);
}

IndexVector find_ceil_string(const Rcpp::CharacterVector& vec, const Rcpp::CharacterVector& value) {
    return find_ceil(vec, value);
}

IndexVector find_floor_number(const Rcpp::NumericVector& vec, const Rcpp::NumericVector& value) {
    return find_floor(vec, value);
}

IndexVector find_floor_string(const Rcpp::CharacterVector& vec, const Rcpp::CharacterVector& value) {
    return find_floor(vec, value);
}

IndexVector find_floor_reverse_string(const Rcpp::CharacterVector& vec, const Rcpp::CharacterVector& value) {
    return find_floor_reverse(vec, value);
}
#endif

#ifdef TESTING_FROM_MAIN
template <typename T, typename F>
int test_common(const T& vec, const T& keys, const IndexVector& expected, F& tested_func) {
    const auto resultSize = keys.size();
    const auto actual = tested_func(vec, keys);
    if ((actual.size() != resultSize) || (expected.size() != resultSize) || (resultSize == 0)) {
        return 1;
    }

    for(IndexVector::size_type i = 0; i < resultSize; ++i) {
        if (actual.at(i) != expected.at(i)) {
            return 1;
        }
    }
    return 0;
}

int test_one_element_between_fences(void) {
    int result = 0;
    const Rcpp::NumericVector vec_number {1.0, 2.0, 3.0, 4.0};
    const Rcpp::NumericVector keys_number {0.5, 1.5, 2.5, 3.5, 4.5};
    const IndexVector expectedCeil {1, 2, 3, 4, 0};
    const IndexVector expectedFloor {0, 1, 2, 3, 4};
    result |= test_common(vec_number, keys_number, expectedCeil, find_ceil_number);
    result |= test_common(vec_number, keys_number, expectedFloor,  find_floor_number);

    const Rcpp::CharacterVector vec_string {"B", "BD", "BDF", "D"};
    const Rcpp::CharacterVector keys_string {"A", "BC", "BDE", "C", "E"};
    result |= test_common(vec_string, keys_string, expectedCeil, find_ceil_string);
    result |= test_common(vec_string, keys_string, expectedFloor, find_floor_string);
    return result;
}

int test_two_elements_between_fences(void) {
    int result = 0;
    const Rcpp::NumericVector vec_number {5.0, 6.0, 7.0, 8.0};
    const Rcpp::NumericVector keys_number {0.5, 5.5, 5.6, 6.5, 8.5, 8.6};
    const IndexVector expectedCeil {1, 2, 2, 3, 0, 0};
    const IndexVector expectedFloor {0, 1, 1, 2, 4, 4};
    result |= test_common(vec_number, keys_number, expectedCeil, find_ceil_number);
    result |= test_common(vec_number, keys_number, expectedFloor, find_floor_number);
    return result;
}

int test_exact_elements(void) {
    int result = 0;
    const Rcpp::CharacterVector vec_string {"B", "BC", "C", "D"};
    const Rcpp::CharacterVector keys_string {"A", "BC", "BCD", "CA", "D", "DE"};
    const IndexVector expectedCeil {1, 2, 3, 4, 4, 0};
    const IndexVector expectedFloor {0, 2, 2, 3, 4, 4};
    result |= test_common(vec_string, keys_string, expectedCeil, find_ceil_string);
    result |= test_common(vec_string, keys_string, expectedFloor, find_floor_string);
    return result;
}

int test_floor_alt(void) {
    int result = 0;
    const Rcpp::CharacterVector vec_string {"D", "C", "BC", "B"};
    const Rcpp::CharacterVector keys_string {"DE", "D", "CA", "BCD", "BC", "A"};
    const IndexVector expectedFloor {1, 1, 2, 3, 3, 0};
    result |= test_common(vec_string, keys_string, expectedFloor, find_floor_reverse_string);
    return result;
}

int main(int argc, char* argv[]) {
    std::vector<char> charSet;
    signed char c = 0x7f;
    do {
        charSet.push_back(c);
    } while(--c >= 0x20);

    std::sort(charSet.begin(), charSet.end());
    for(const auto c : charSet) {
        std::cout << c;
    }

    auto result = test_one_element_between_fences();
    result |= test_two_elements_between_fences();
    result |= test_exact_elements();
    result |= test_floor_alt();
    if (!result) {
        std::cout << "Everything in OK!\n";
    }

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
