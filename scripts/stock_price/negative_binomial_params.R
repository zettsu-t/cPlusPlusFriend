library(ggplot2)
library(reshape2)
library(rstan)
library(plyr)
library(dplyr)
library(purrr)

xs <- 0:100
use_stan <- FALSE

## https://stackoverflow.com/questions/10397574/efficiently-compute-mean-and-standard-deviation-from-a-frequency-table
table_to_sd_text <- function(xs, counts) {
    s <- sum(counts)
    mu <- sum(xs * counts) / s
    sigma <- sqrt(sum((xs - mu)**2 * counts) / (s - 1))
    return (list(mu=mu, sigma=sigma))
}

near_zero <- function(x) {
    ifelse(x > -1e-03 && x < 1e-03, 0, x)
}

sapply(c(0.25, 0.4, 0.5, 0.6, 0.75), function(actual_prob) {
    size_set <- c(1, 2, 3, 4, 5, 8, 10, 12, 15)
    df <- lapply(size_set, function(actual_size) {
        df <- data.frame(x=xs)
        ys <- dnbinom(xs, size=actual_size, prob=actual_prob)
        col <- paste(sprintf('size=%.3f', actual_size), sprintf('prob=%.3f', actual_prob), sep=' ')
        df[[col]] <- ys

        mu <- actual_size * (1 - actual_prob) / actual_prob
        col <- paste(sprintf('size=%.3f', actual_size), sprintf('mu=%.3f', mu), sep=' ')
        df[[col]] <- dnbinom(xs, size=actual_size, mu=mu) * 1.01

        y_sd <- sqrt(actual_size * (1 - actual_prob) / (actual_prob * actual_prob))
        mean_sd <- table_to_sd_text(xs, ys * 100000)
        print(paste("mu diff", near_zero(mu - mean_sd$mu)))
        print(paste("sd diff", near_zero(y_sd - mean_sd$sigma)))

        if (use_stan) {
            input_data <- list(N=length(ys), M=10000, Y=as.integer(ys*100000))
            fit <- stan(file='negative_binomial_params.stan', data=input_data, iter=500, warmup=250, chains=4)
            fit.df <- data.frame(get_posterior_mean(fit))
            alpha <- fit.df['alpha','mean.all.chains']
            beta <- fit.df['beta','mean.all.chains']
            estimated_prob <- beta / (1 + beta)

            col <- paste(sprintf('Stan estimation size=%.3f', actual_size), sprintf('prob=%.3f', estimated_prob), sep=' ')
            df[[col]] <- dnbinom(xs, size=actual_size, prob=actual_prob) * 0.99
        }
        df
    }) %>% reduce(left_join, by='x')
    df.melt <- melt(df, id.vars='x')

    filename <- paste(gsub('0\\.', '', sprintf('prob_%.3f', actual_prob)), '.png', sep='')
    png(filename=filename, width=1200, height=600)
    g <- ggplot(df.melt, aes(x=x, y=value, fill=variable, colour=variable))
    g <- g + geom_line() ## linetype=rep(c('dotted','dashed'), length(size_set)))
    g <- g + xlab('X')
    g <- g + ylab('Density')
    g <- g + xlim(0, 40)
    g <- g + theme(legend.position=c(0.8, 0.75))
    g <- g + guides(col = guide_legend(ncol=3))
    g <- g + labs(color='Parameter set')
    plot(g)
    dev.off()
    TRUE
})
