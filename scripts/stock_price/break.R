library(Rcpp)
library(BH)

Sys.setenv("PKG_CXXFLAGS"="-std=gnu++14")
sourceCpp('break.cpp')
calc_long_time(1)
calc_long_time(10000)
