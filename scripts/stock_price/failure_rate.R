library(ggplot2)
library(gridExtra)
library(reshape2)
library(rstan)
library(matrixStats)

common_seed <- 123456
stan_warmup <- 1000000
stan_iter <- 1000000
n_stan_chains <- 1
n <- 80
input_data <- list(N=n)
fit.stan <- stan(file='failure_rate.stan', data=input_data,
                 iter=stan_iter+stan_warmup, warmup=stan_warmup,
                 chains=n_stan_chains, seed=common_seed)
summary(fit.stan)
fit_extracted <- extract(fit.stan)
samples <- fit_extracted$p

make_dataset <- function(x_max) {
    x_minor_step <- 0.000001
    x_major_step <- 0.0001
    x_thredhold <- 0.002
    if (x_max > x_thredhold + x_major_step) {
        xs <- seq(0.0, x_max - x_major_step, x_major_step)
    } else {
        xs <- seq(0.0, x_max - x_minor_step, x_minor_step)
    }

    densities <- binCounts(samples, bx=c(xs, x_max))
    densities <- densities / NROW(samples)
    cumulative_densities <- cumsum(densities)
    calculated_probabilities <- sapply(xs, function(p) { 1.0 - ((1.0 - p) ** n) })
    diff_probabilities <- c(diff(calculated_probabilities), NA)

    ## Converts to percentages
    xs <- xs * 100
    return (list(xs=xs, densities=densities,
                 cumulative_densities=cumulative_densities,
                 calculated_probabilities=calculated_probabilities,
                 diff_probabilities=diff_probabilities))
}

draw_all <- function(dataset) {
    g1 <- ggplot()
    g1 <- g1 + geom_line(aes(x=dataset$xs, y=dataset$densities), linetype='solid', color='navy', size=1.5)
    g1 <- g1 + ggtitle('Probability densities : MCMC samples (# of failures > 0 in 80 pieces)')
    g1 <- g1 + xlab('Failure rate per piece [%]')
    g1 <- g1 + ylab('Density')

    g2 <- ggplot()
    g2 <- g2 + geom_line(aes(x=dataset$xs, y=dataset$diff_probabilities), linetype='solid', color='navy', size=1.5)
    g2 <- g2 + ggtitle('Probability densities calculated (# of failures > 0 in 80 pieces)')
    g2 <- g2 + xlab('Failure rate per piece [%]')
    g2 <- g2 + ylab('Density')

    g3 <- ggplot()
    g3 <- g3 + geom_line(aes(x=dataset$xs, y=dataset$cumulative_densities), linetype='solid', color='darkorange', size=1.5)
    g3 <- g3 + ggtitle('Cumulative probability densities : MCMC samples (# of failures > 0 in 80 pieces)')
    g3 <- g3 + xlab('Failure rate per piece [%]')
    g3 <- g3 + ylab('Probability')

    g4 <- ggplot()
    g4 <- g4 + geom_line(aes(x=dataset$xs, y=dataset$calculated_probabilities), linetype='solid', color='darkorange', size=1.5)
    g4 <- g4 + ggtitle('Calculated Probabilities (# of failures > 0 in 80 pieces)')
    g4 <- g4 + xlab('Failure rate per piece [%]')
    g4 <- g4 + ylab('Probability')

    grid.arrange(g1, g2, g3, g4)
}

draw_probabilities <- function(dataset) {
    df <- data.frame(x=dataset$xs, densities=dataset$densities, probabilities=dataset$diff_probabilities)
    df.melt <- melt(df, id.vars='x')
    g <- ggplot(df.melt, aes(x=x, y=value, color=variable))
    g <- g + geom_line(size=1.0, alpha=0.8)
    g <- g + ggtitle('Probability (# of failures > 0 in 80 pieces)')
    g <- g + scale_color_manual(labels=c('MCMC samples',
                                         'Differences of calculated probabilities'),
                                values=c('navy', 'darkorange'))
    g <- g + xlab('Failure rate per piece [%]')
    g <- g + ylab('Density')
    g <- g + theme(legend.position=c(0.6, 0.85), legend.title=element_blank())
    g <- g + guides(col = guide_legend(ncol=1))
    plot(g)
}

draw_cumulative_probabilities <- function(dataset) {
    df <- data.frame(x=dataset$xs, densities=dataset$cumulative_densities, probabilities=dataset$calculated_probabilities)
    df.melt <- melt(df, id.vars='x')
    g <- ggplot(df.melt, aes(x=x, y=value, color=variable))
    g <- g + ggtitle('Cumulative probability densities (# of failures > 0 in 80 pieces)')
    g <- g + geom_line(size=1.0, alpha=0.8)
    g <- g + scale_color_manual(labels=c('MCMC samples',
                                         'Calculated probabilities'),
                                values=c('navy', 'darkorange'))
    g <- g + xlab('Failure rate per piece [%]')
    g <- g + ylab('Probability')
    g <- g + theme(legend.position=c(0.8, 0.25), legend.title=element_blank())
    g <- g + guides(col = guide_legend(ncol=1))
    plot(g)
}

dataset <- make_dataset(0.03)

png(filename='failure_rate_all.png', width=1600, height=1200)
draw_all(dataset)
dev.off()

png(filename='failure_rate_prob.png', width=800, height=600)
draw_probabilities(dataset)
dev.off()

png(filename='failure_rate_cum.png', width=800, height=600)
draw_cumulative_probabilities(dataset)
dev.off()

dataset <- make_dataset(0.0005)
png(filename='failure_rate_small.png', width=800, height=600)
draw_cumulative_probabilities(dataset)
dev.off()
