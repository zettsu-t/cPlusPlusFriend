library(ggplot2)
library(reshape2)
library(rstan)

actual_mu_base <- 10
actual_mu_trend <- 4.5
actual_variance_base <- 1.0
actual_variance_trend <- 0.2
n_steps <- 5
n_samples <- 1000

calculate_size_mu <- function(mu_base, mu_trend, variance_base, variance_trend, scale) {
    mu <- mu_trend * scale + mu_base
    variance_nb <- (1.0 + variance_trend) * mu + variance_base
    ## from the Stan reference manual
    prob <- mu / variance_nb
    size <- mu * prob / (1.0 - prob)
    return (list(size=size, mu=mu))
}

calculate_size_prob <- function(alpha_base, alpha_trend, beta_base, beta_trend, scale) {
    alpha <- alpha_trend * scale + alpha_base
    beta <- beta_trend * scale + beta_base
    prob <- beta / (1.0 + beta)
    return (list(size=alpha, prob=prob))
}

calculate_ab <- function(mu, variance) {
    ## mu = alpha / beta
    ## variance = alpha * (beta + 1) / beta^2
    ## variance / mu = (beta + 1) / beta
    beta <- 1 / (variance / mu - 1)
    alpha <- mu / beta
    return (list(alpha=alpha, beta=beta))
}

ab_set <- data.frame(t(sapply(1:n_steps, function(scale) {
    mu <- actual_mu_trend * scale + actual_mu_base
    variance_nb <- (1.0 + actual_variance_trend) * mu + actual_variance_base
    param_set <- calculate_ab(mu, variance_nb)
})))

ab_set$alpha <- as.numeric(ab_set$alpha)
ab_set$beta <- as.numeric(ab_set$beta)

g <- ggplot(data=ab_set)
g <- g + geom_line(aes(x=alpha, y=beta))
plot(g)

## Makes observations as random numbers
observed <- NULL
for(scale in 1:n_steps) {
    param_set <- calculate_size_mu(actual_mu_base, actual_mu_trend,
                                   actual_variance_base, actual_variance_trend, scale)
    ys <- data.frame(rnbinom(n=n_samples, size=param_set$size, mu=param_set$mu))
    if (is.null(observed)) {
        observed <- ys
    } else {
        observed <- cbind(observed, ys)
    }
}
names(observed) <- 1:n_steps
max_x <- max(observed)

## Converts the observations to density histograms
densities <- NULL
for(scale in 1:n_steps) {
    ## 0-based
    density <- data.frame(tabulate(1 + observed[,scale], max_x + 1)) / n_samples
    names(density) <- sprintf("%2dh", scale)
    if (is.null(densities)) {
        densities <- density
    } else {
        densities <- cbind(densities, density)
    }
}
densities$x <- 0:max_x

## Fits on Stan and gets mean estimations of the model 1 (alpha-beta
input_data <- list(K=n_steps, N=n_samples, Y=observed)
fit1 <- stan(file='negative_binomial_fit1.stan', data=input_data, iter=200, warmup=100, chains=2)
fit1.df <- data.frame(get_posterior_mean(fit1))
mean_alpha_base <- fit1.df['alpha_base','mean.all.chains']
mean_alpha_trend <- fit1.df['alpha_trend','mean.all.chains']
mean_beta_base <- fit1.df['beta_base','mean.all.chains']
mean_beta_trend <- fit1.df['beta_trend','mean.all.chains']

## Makes predictions under the model
predicted1 <- NULL
for(scale in 1:n_steps) {
    param_set <- calculate_size_prob(mean_alpha_base, mean_alpha_trend,
                                     mean_beta_base, mean_beta_trend, scale)
    ys <- data.frame(dnbinom(0:max_x, size=param_set$size, prob=param_set$prob))
    if (is.null(predicted1)) {
        predicted1 <- ys
    } else {
        predicted1<- cbind(predicted1, ys)
    }
}
names(predicted1) <- unlist(sapply(1:n_steps, function(i) { sprintf("%2dp1", i) }))
predicted1$x <- 0:max_x

## Fits on Stan and gets mean estimations of the model 2 (mu-variance)
input_data <- list(K=n_steps, N=n_samples, Y=observed)
fit2 <- stan(file='negative_binomial_fit2.stan', data=input_data, iter=200, warmup=100, chains=2)
fit2.df <- data.frame(get_posterior_mean(fit2))
mean_mu_base <- fit2.df['mu_base','mean.all.chains']
mean_mu_trend <- fit2.df['mu_trend','mean.all.chains']
mean_variance_base <- fit2.df['variance_base','mean.all.chains']
mean_variance_trend <- fit2.df['variance_trend','mean.all.chains']

## Makes predictions under the model
predicted2 <- NULL
for(scale in 1:n_steps) {
    param_set <- calculate_size_mu(mean_mu_base, mean_mu_trend,
                                   mean_variance_base, mean_variance_trend, scale)
    ys <- data.frame(dnbinom(0:max_x, size=param_set$size, mu=param_set$mu))
    if (is.null(predicted2)) {
        predicted2 <- ys
    } else {
        predicted2 <- cbind(predicted2, ys)
    }
}
names(predicted2) <- unlist(sapply(1:n_steps, function(i) { sprintf("%2dp2", i) }))
predicted2$x <- 0:max_x

predicted1.melt <- melt(predicted1, id.vars='x')
predicted2.melt <- melt(predicted2, id.vars='x')
densities.melt <- melt(densities, id.vars='x')
g <- ggplot()
g <- g + geom_line(data=predicted1.melt, aes(x=x, y=value, colour=variable), linetype='dotted')
g <- g + geom_line(data=predicted2.melt, aes(x=x, y=value, colour=variable), linetype='dashed')
g <- g + geom_line(data=densities.melt, aes(x=x, y=value, colour=variable), linetype='solid')
g <- g + theme(legend.position='none')
plot(g)
