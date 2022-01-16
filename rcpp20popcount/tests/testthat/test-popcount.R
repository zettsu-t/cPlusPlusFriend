Sys.setlocale("LC_ALL", "C")
options(future.globals.onMissing = "ignore")

test_that("Popcount", {
  xs <- c(0:8, 65535, 65536, 2147483646, 2147483647)
  expected <- c(0, 1, 1, 2, 1, 2, 2, 3, 1, 16, 1, 30, 31)
  actual <- rcpp20popcount::popcount(xs)
  expect_equal(object = actual, expected = expected)
})
