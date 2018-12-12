library(ggplot2)
library(reshape2)
library(rstan)

xs <- 0:50
for(actual_prob in c(0.25, 0.4, 0.5, 0.6, 0.75)) {
    df <- data.frame(x=xs)

    size_set <- c(1, 2, 3, 4, 5, 8, 10, 12, 15)
    for(actual_size in size_set) {
        ys <- dnbinom(xs, size=actual_size, prob=actual_prob)
        col <- paste(sprintf('size=%.3f', actual_size), sprintf('prob=%.3f', actual_prob), sep=' ')
        df[[col]] <- ys

        mu <- actual_size * (1 - actual_prob) / actual_prob
        col <- paste(sprintf('size=%.3f', actual_size), sprintf('mu=%.3f', mu), sep=' ')
        df[[col]] <- dnbinom(xs, size=actual_size, prob=actual_prob) * 1.01

        input_data <- list(N=length(ys), M=10000, Y=as.integer(ys*100000))
        fit <- stan(file='negative_binomial_params.stan', data=input_data, iter=1000, warmup=500, chains=4)
        fit.df <- data.frame(get_posterior_mean(fit))
        alpha <- fit.df['alpha','mean.all.chains']
        beta <- fit.df['beta','mean.all.chains']
        estimated_prob <- beta / (1 + beta)

        col <- paste(sprintf('Stan estimation size=%.3f', actual_size), sprintf('prob=%.3f', estimated_prob), sep=' ')
        df[[col]] <- dnbinom(xs, size=actual_size, prob=actual_prob) * 0.99
    }
    df.melt <- melt(df, id.vars='x')

    filename <- paste(gsub('0\\.', '', sprintf('size_%.3f', actual_prob)), '.png', sep='')
    png(filename=filename, width=1200, height=600)
    g <- ggplot(df.melt, aes(x=x, y=value, fill=variable, colour=variable))
    g <- g + geom_line() ## linetype=rep(c('dotted','dashed'), length(size_set)))
    g <- g + xlab('X')
    g <- g + ylab('Density')
    g <- g + theme(legend.position=c(0.8, 0.75))
    g <- g + labs(color='Parameter set')
    plot(g)
    dev.off()
}
