Sys.setlocale("LC_ALL", "C")
options(future.globals.onMissing = "ignore")

test_that("Popcount", {
  xs <- 0:8
  expected <- c(0, 1, 1, 2, 1, 2, 2, 3, 1)
  actual <- rcpp20popcount::popcount(xs)
  expect_equal(object = actual, expected = expected)
})
