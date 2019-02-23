library(ggplot2)
library(data.table)
library(plyr)
library(dplyr)

## Density from 0 to n (n+1 elements)
merge_density <- function(density_left, density_right) {
    max_left <- length(density_left) - 1
    max_right <- length(density_right) - 1
    max_joint <- max_left + max_right

    rowSums(sapply(0:max_right, function(index) {
        ## 1-based index
        density <- density_right[index + 1]
        ## joint probability
        likelihoods <- density_left * density
        n_fill <- max_joint - max_left - index
        c(rep(0, index), likelihoods, rep(0, n_fill))
    }))
}

plot_density <- function(density_left, density_right, expected_var) {
    vec <- merge_density(density_left, density_right)
    xs <- 0:(NROW(vec)-1)
    g <- ggplot()
    g <- g + geom_line(aes(x=xs, y=vec),
                       color="navy", alpha=0.8, linetype="solid")
    g <- g + geom_line(aes(x=xs, y=dnorm(xs, max(xs)/2+0.1, sqrt(expected_var))),
                       color="violet", alpha=0.8, linetype="dashed")
    plot(g)

    print(sum(vec))
    df <- data.table(count=0:(NROW(vec)-1), freq=vec)
    df_acc <- rep(df$count, df$freq * 10000000)
    print(mean(df_acc))
    print(var(df_acc))
    vec
}

vec <- plot_density(dnorm(0:40, 20, 3), dnorm(0:80, 40, 7), 58)
vec <- plot_density(vec, dnorm(0:80, 40, 5), 83)
