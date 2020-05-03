suppressWarnings(suppressMessages(library("microbenchmark")))
suppressWarnings(suppressMessages(library("purrr")))
context("test all")

## A naive implementation
base_popcount <- function(x) {
    bits <- 2 ** seq(from=0, to=30)
    purrr::map_int(.x=x, .f= ~ sum(bitwAnd(.x, bits) > 0))
}

test_that("zero", {
    x <- integer()
    expected <- integer()
    expect_equal(object=popcountr::popcountr(x), expected=expected)
    expect_equal(object=base_popcount(x), expected=expected)
})

test_that("Int Max", {
    x <- c(2147483647, 2147483646)
    expected <- c(31, 30)
    expect_equal(object=popcountr::popcountr(x), expected=expected)
    expect_equal(object=base_popcount(x), expected=expected)
})

test_that("one", {
    x <- c(5)
    expected <- c(2)
    expect_equal(object=popcountr::popcountr(x), expected=expected)
    expect_equal(object=base_popcount(x), expected=expected)
})

test_that("two", {
    x <- c(5, 19)
    expected <- c(2, 3)
    expect_equal(object=popcountr::popcountr(x), expected=expected)
    expect_equal(object=base_popcount(x), expected=expected)
})

test_that("NA", {
    x <- c(15, NA, 31)
    expected <- c(4, NA, 5)
    expect_equal(object=popcountr::popcountr(x), expected=expected)
    expect_equal(object=base_popcount(x), expected=expected)
})

test_that("long", {
    bits <- seq(from=0, to=29)
    pattern <- 2 ** bits
    n_unit <- 1000
    x <- rep(x=pattern, times=n_unit)
    expect_equal(object=sum(popcountr::popcountr(x)), expected=NROW(x))

    pattern <- cumsum(pattern)
    x <- rep(x=c(0, NA, pattern), times=n_unit)
    expected <- rep(x=c(0, NA, bits + 1), times=n_unit)
    expect_equal(object=popcountr::popcountr(x), expected=expected)
})

test_that("time", {
    skip_if_not(interactive())
    x <- sample(1:2147483647, 100000, replace=TRUE)
    microbenchmark::microbenchmark(popcountr(x), unit="ms")
    microbenchmark::microbenchmark(base_popcount(x), times=10, unit="ms")
})
