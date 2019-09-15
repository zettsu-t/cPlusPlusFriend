#include <boost/math/distributions/poisson.hpp>
#include <iostream>

// C++17 allows us to omit tail template parameters.
template<template<class>class T, typename... U>
auto f(double q, U&&... params) {
    T<double> dist(params...);
    return boost::math::quantile(dist, q);
}

// C++14 requires all explicit template parameters.
template<template<class,class>class T, typename... U>
auto g(double q, U&&... params) {
    T<double, boost::math::policies::policy<>> dist(params...);
    return boost::math::quantile(dist, q);
}

void compilationError(void) {
//  This may cause error message which is hard to understand.
//  int policy = 0;
    using namespace boost::math::policies;
//  This "policy" is namespace boost::math::policies::policy, not a local variable.
    boost::math::poisson_distribution<double, policy<discrete_quantile<integer_round_up>>> dist;
}

int main(int argc, char* argv[]) {
    using namespace boost::math;
    std::cout << f<boost::math::poisson_distribution>(0.9, 2.5) << "\n";
    std::cout << g<boost::math::poisson_distribution>(0.9, 2.5) << "\n";
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
