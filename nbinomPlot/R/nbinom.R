#' A Shiny app to plot negative binomial distributions interactively

## Default UI settings
#' The default max value of the slide input for size parameters of negative binomial distributions
default_max_size <- 40

#' The default value of the slide input for size parameters of negative binomial distributions
default_size <- 1.0

#' The default value of the slide input for prob parameters of negative binomial distributions
default_prob <- 0.5

#' Calculate the mu of a negative binomial distribution its size and prob parameters
#'
#' @param size Size parameter of a negative binomial distribution
#' @param prob Prob parameter of a negative binomial distribution
calculate_nbinom_mu_from_size_prob <- function(size, prob) {
  size * (1 - prob) / prob
}

#' Calculate the size of a negative binomial distribution its prob and mu parameters
#'
#' @param prob Prob parameter of a negative binomial distribution
#' @param mu Mu parameter of a negative binomial distribution
calculate_nbinom_size_from_prob_mu <- function(prob, mu) {
  mu * prob / (1 - prob)
}
