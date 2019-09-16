library(purrrlyr)
library(tibble)
library(plyr)
library(dplyr)
library(SDMTools)

joint_prob <- function(ps, qs) {
    df <- tibble(n=0:(NROW(ps)-1), p=ps)
    x <- purrrlyr::by_row(df, function(row) {
        left_n <- row$n
        right_n <- NROW(ps) - 1 - left_n
        c(rep(0, left_n), qs * row$p, rep(0, right_n))
    })$.out %>% plyr::ldply()

    x %>% colSums()
}

xs <- 0:30
ys <- joint_prob(dpois(xs, 3), dnorm(xs, 5))
df <- tibble(x=0:(NROW(ys)-1), p=ys)
SDMTools::wt.mean(df$x, df$p)
