#' Fast population count
#'
#' Considerably fast population count using C++ compiler extensions and CPU instructions.
#' @useDynLib popcountr popcountr_impl
#' @importFrom Rcpp sourceCpp
NULL

#' Fast population count
#'
#' Considerably fast population count using C++ compiler extensions and CPU instructions.
#' @useDynLib popcountr popcountr_impl
#' @param x A single non-negative integer vector which can contain NA_integer_.
#'   Results for negative integers are undefined.
#' @return Population counts of each x.
#' @examples
#' popcountr(5)
#' popcountr(2147483647)
#' popcountr(c(5, 19))
#' popcountr(c(15, NA, 31))
#' @export
popcountr <- function(x) {
    na_set <- is.na(x)
    ##  This does not work.
    ##  .Call('popcountr_impl', x)
    count_set <- popcountr_(x)

    stopifnot(length(na_set) == length(x))
    stopifnot(length(na_set) == length(count_set))
    count_set[na_set] <- NA
    count_set
}

.onUnload <- function (libpath) {
    library.dynam.unload("popcountr", libpath)
}
