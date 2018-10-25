## Based on
## http://norimune.net/2949
## http://kosugitti.sakura.ne.jp/wp/wp-content/uploads/2013/08/irtnote.pdf
## http://www.ieice-hbkb.org/files/S3/S3gun_11hen_03.pdf

library(dplyr)
library(ggmcmc)
library(ggplot2)
library(raster)
library(rstan)
library(boot)
library(Rlab)
library(reshape2)
library(stringr)
library(tidyverse)

common_seed <- 123
n <- 1500
k <- 24

## Assuming four-choice questions
chance <- 0.25

mu_theta <- 0.5
sigma_theta <- 0.18
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

png(filename='itr_probability.png', width=1200, height=600)
g <- ggplot(data.df, aes(x=x, y=value))
g <- g + geom_line(aes(colour=difficulty), size=2)
g <- g + theme(legend.position="none")
g <- g + xlab('Difficulty')
g <- g + ylab('Probability to choose correct answers')
g <- g + theme(axis.text=element_text(size=24), axis.title=element_text(size=24), plot.title=element_text(size=24))
plot(g)
dev.off()

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

png(filename='irt_difficulty_hist.png', width=1200, height=800)
stan_hist(fit.stan, pars=c('difficulty'), nrow=4, ncol=6)
dev.off()

df_fit <- as.data.frame(get_posterior_mean(fit.stan))
names(df_fit) <- c('estimation')
estimated_diffs <- df_fit[grep('^difficulty', rownames(df_fit)),]
df.difficulty <- data.frame(index=1:length(estimated_diffs), estimated=estimated_diffs, actual=difficulties)
df.difficulty$diff <- df.difficulty$estimated - df.difficulty$actual

png(filename='itr_difficulty_hist.png', width=1200, height=800)
g <- ggplot(df.difficulty)
g <- g + geom_histogram(aes(x=diff), bins=7)
g <- g + labs(title='Difference between estimated and actual difficulty')
g <- g + xlab('Difference')
g <- g + ylab('Frequency')
g <- g + theme(axis.text=element_text(size=24), axis.title=element_text(size=24), plot.title=element_text(size=24))
plot(g)
dev.off()

png(filename='itr_difficulty_plot.png', width=1200, height=800)
g <- ggplot(df.difficulty)
g <- g + geom_line(aes(x=index, y=actual), color='black', size=2)
g <- g + geom_point(aes(x=index, y=estimated), color='orchid', size=5)
g <- g + labs(title='Estimated and actual difficulty')
g <- g + xlab('Index')
g <- g + ylab('Difficulty')
g <- g + theme(axis.text=element_text(size=24), axis.title=element_text(size=24), plot.title=element_text(size=24))
plot(g)
dev.off()

estimated_theta <- df_fit[grep('^theta', rownames(df_fit)),]
df.theta <- data.frame(estimated=estimated_theta, actual=theta)
df.theta <- df.theta[order(df.theta$actual, df.theta$estimated),]
df.theta$index <- 1:nrow(df.theta)
df.theta$diff <- df.theta$estimated - df.theta$actual

png(filename='itr_theta_hist.png', width=1200, height=800)
g <- ggplot(df.theta)
g <- g + geom_histogram(aes(x=diff))
g <- g + labs(title='Difference between estimated and actual ability')
g <- g + xlab('Difference')
g <- g + ylab('Frequency')
g <- g + theme(axis.text=element_text(size=24), axis.title=element_text(size=24), plot.title=element_text(size=24))
plot(g)
dev.off()

png(filename='itr_theta_plot.png', width=1200, height=800)
g <- ggplot(df.theta)
g <- g + geom_point(aes(x=index, y=estimated), color='orchid', size=3)
g <- g + geom_line(aes(x=index, y=actual), color='black', size=2)
g <- g + labs(title='Estimated and actual ability')
g <- g + xlab('Index')
g <- g + ylab('Ability')
g <- g + theme(axis.text=element_text(size=24), axis.title=element_text(size=24), plot.title=element_text(size=24))
plot(g)
dev.off()
