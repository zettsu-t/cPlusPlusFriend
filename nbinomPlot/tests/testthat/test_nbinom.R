library(nbinomPlot)
context("test nbinomPlot")

test_that("calculate_nbinom_mu_from_size_prob", {
  expect_equal(object = calculate_nbinom_mu_from_size_prob(size = 1.0, prob = 0.5), expected = 1.0, tolerance = 1e-7)
  expect_equal(object = calculate_nbinom_mu_from_size_prob(size = 4.0, prob = 0.25), expected = 12.0, tolerance = 1e-7)
  expect_equal(object = calculate_nbinom_mu_from_size_prob(size = 6.0, prob = 0.75), expected = 2.0, tolerance = 1e-7)
})

test_that("calculate_nbinom_size_from_prob_mu", {
  expect_equal(object = calculate_nbinom_size_from_prob_mu(prob = 0.5, mu = 1.0), expected = 1.0, tolerance = 1e-7)
  expect_equal(object = calculate_nbinom_size_from_prob_mu(prob = 0.25, mu = 12.0), expected = 4.0, tolerance = 1e-7)
  expect_equal(object = calculate_nbinom_size_from_prob_mu(prob = 0.75, mu = 2.0), expected = 6.0, tolerance = 1e-7)
})

test_that("density_nbinom_mu_from_size_prob", {
  xs <- 0:30
  size <- 4.0
  prob <- 0.25
  mu <- 12.0
  expected <- dnbinom(xs, size = size, prob = prob)
  actual <- dnbinom(xs, size = size, mu = mu)
  expect_equal(object = actual, expected = expected, tolerance = 1e-5)
})
