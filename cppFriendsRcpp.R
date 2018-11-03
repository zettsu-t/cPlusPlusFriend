library(Rcpp)
library(BH)

Sys.setenv("PKG_CXXFLAGS"="-std=gnu++14")
sourceCpp('cppFriendsRcpp.cpp')
split_double_components(c(0.0, 1.0, 1.5, -3.5, Inf, -Inf))
