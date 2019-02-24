library(ggplot2)
library(data.table)
library(plyr)
library(dplyr)

## Density from 0 to n (n+1 elements)
merge_density <- function(density_left, density_right) {
    max_left <- length(density_left) - 1
    max_right <- length(density_right) - 1
    max_joint <- max_left + max_right

    vec <- rowSums(sapply(0:max_right, function(index) {
        ## 1-based index
        density <- density_right[index + 1]
        ## joint probability
        likelihoods <- density_left * density
        n_fill <- max_joint - max_left - index
        c(rep(0, index), likelihoods, rep(0, n_fill))
    }))

    print(sum(vec))
    df <- data.table(count=0:(NROW(vec)-1), freq=vec)
    df_acc <- rep(df$count, df$freq * 10000000)
    print(mean(df_acc))
    print(var(df_acc))
    vec
}

plot_norm_density <- function(vec, expected_mean, expected_var) {
    xs <- 0:(NROW(vec)-1)
    g <- ggplot()
    g <- g + geom_line(aes(x=xs, y=vec),
                       color="navy", alpha=0.8, linetype="solid")
    g <- g + geom_line(aes(x=xs, y=dnorm(xs, expected_mean, sqrt(expected_var))),
                       color="violet", alpha=0.8, linetype="dashed")
    plot(g)
}

plot_pois_density <- function(vec, expected_mean) {
    xs <- 0:(NROW(vec)-1)
    g <- ggplot()
    g <- g + geom_line(aes(x=xs, y=vec),
                       color="navy", alpha=0.8, linetype="solid")
    g <- g + geom_line(aes(x=xs, y=dpois(xs, expected_mean)),
                       color="violet", alpha=0.8, linetype="dashed")
    plot(g)
}

plot_density <- function(vec, max_x) {
    n <- min(max_x + 1, NROW(vec))
    xs <- 0:(n-1)
    ys <- vec[1:n]
    g <- ggplot()
    g <- g + geom_line(aes(x=xs, y=ys), color="navy", alpha=0.8, linetype="solid")
    plot(g)
}

vec <- merge_density(dnorm(0:40, 20, 3), dnorm(0:80, 40, 7))
plot_norm_density(vec, 60, 58)
vec <- merge_density(vec, dnorm(0:80, 40, 5))
plot_norm_density(vec, 100, 83)

vec <- merge_density(dpois(0:30, 3), dpois(0:30, 4))
plot_pois_density(vec, 7)
vec <- merge_density(dpois(0:30, 3), dpois(0:30, 8))
plot_pois_density(vec, 11)

vec <- merge_density(dnbinom(0:100, size=10, prob=0.3), dnbinom(0:100, size=15, prob=0.8))
vec <- merge_density(vec, dnbinom(0:100, size=30, prob=0.6))
vec <- merge_density(vec, dnbinom(0:100, size=40, prob=0.55))
plot_density(vec, 150)
