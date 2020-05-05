// [[Rcpp::depends(BH)]]
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
double calc_long_time(int n_loops) {
    double accum = 0.0;

    for(int i=0; i<n_loops; ++i) {
        for(int j=0; j<1000000; ++j) {
            accum += runif(1.0)[0] - 0.5;
        }
        // Need to break from the RStudio console
        checkUserInterrupt();
    }

    return accum;
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
