#' Count populations (# of 1-bits) of integers
#' @param xs An integer vector
#' @return The population (# of 1-bits) of each integer
#' @useDynLib rcpp20popcount, .registration=TRUE
#' @export
#' @importFrom Rcpp sourceCpp
popcount <- function(xs) {
  cpp_popcount(round(xs))
}
