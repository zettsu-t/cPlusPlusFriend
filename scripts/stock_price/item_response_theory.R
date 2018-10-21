## Based on
## http://norimune.net/2949
## http://kosugitti.sakura.ne.jp/wp/wp-content/uploads/2013/08/irtnote.pdf
## http://www.ieice-hbkb.org/files/S3/S3gun_11hen_03.pdf

library(ggplot2)
library(raster)
library(rstan)
library(boot)
library(Rlab)
library(reshape2)

common_seed <- 123
n <- 1500
k <- 25

## Assuming four-choice questions
chance <- 0.25

mu_theta <- 0.5
sigma_theta <- 0.2
difficulty_min <- 0.1
difficulty_max <- 0.9
mu_discrimination <- 16.0
sigma_discrimination <- 3.0

theta <- clamp(rnorm(n, mu_theta, sigma_theta), 0.001, 0.999)
difficulties <- seq(difficulty_min, difficulty_max, length.out=k)
discriminations <- clamp(rnorm(n, mu_discrimination, sigma_discrimination), 0.001, mu_discrimination * 2)

data <- matrix(0, n, k)
data.df <- NULL
xs <- seq(0.0, 1.0, 0.01)
for (i in 1:k) {
    prob <- function(x) {
        p_base <- inv.logit(discriminations[i] * (x - difficulties[i]))
        return (chance + (1 - chance) * p_base)
    }
    data[,i] <- sapply(prob(theta), function(x) { rbern(1, x) })

    subdf <- data.frame(sapply(xs, function(x) { prob(x) }))
    names(subdf) <- paste(sprintf('%.5f', difficulties[i]), sep="")
    if (i == 1) {
        data.df <- subdf
    } else {
        data.df <- cbind(data.df, subdf)
    }
}

data.df$x <- xs
data.df <- melt(data.df, id.vars='x', variable.name='difficulty')
g <- ggplot(data.df, aes(x=x, y=value))
g <- g + geom_line(aes(colour=difficulty))
g <- g + theme(legend.position="none")
plot(g)

input_data <- list(N=n, K=k, Y=data, chance=chance)
start_time <- proc.time()
fit.stan <- stan(file='item_response_theory.stan', data=input_data,
                 iter=1000, warmup=500, chains=1, seed=common_seed)
elapsed_time <- proc.time() - start_time

summary(fit.stan)
stan_hist(fit.stan, pars=c('difficulty'))
get_posterior_mean(fit.stan)
print(fit.stan, pars=c('discrimination'))
print(fit.stan, pars=c('difficulty'))
print(elapsed_time)

png(filename='difficulties.png', width=1024, height=1024)
stan_hist(fit.stan, pars=c('difficulty'))
dev.off()
