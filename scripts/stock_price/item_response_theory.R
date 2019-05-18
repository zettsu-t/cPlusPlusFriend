## Based on
## http://norimune.net/2949
## http://kosugitti.sakura.ne.jp/wp/wp-content/uploads/2013/08/irtnote.pdf
## http://www.ieice-hbkb.org/files/S3/S3gun_11hen_03.pdf

library(bayesplot)
library(plyr)
library(dplyr)
library(ggmcmc)
library(ggplot2)
library(gridExtra)
library(ggthemes)
library(raster)
library(rstan)
library(boot)
library(Rlab)
library(reshape2)
library(stringr)

common_seed <- 123

## Set numbers of iterations in testing
n_stan_chains <- 4

## Number of respondents(n) and questions(k)
## stan_warmup <- 250
## stan_iter <- 500
## n <- 1500
## k <- 24

## ggs sample
stan_warmup <- 500
stan_iter <- 500
n <- 4
k <- 100

## Assuming four-choice questions
chance <- 0.25
mu_theta <- 0.5
sigma_theta <- 0.17
difficulty_min <- 0.1
difficulty_max <- 0.9
mu_discrimination <- 16.0
sigma_discrimination <- 3.0

## Generates samples
theta <- clamp(sort(rnorm(n, mu_theta, sigma_theta)), 0.001, 0.999)
difficulties <- seq(difficulty_min, difficulty_max, length.out=k)
discriminations <- clamp(rnorm(k, mu_discrimination, sigma_discrimination), 0.001, mu_discrimination * 2)

## Draws a chart of difficulties of questions
data <- matrix(0, n, k)
xs <- seq(0.0, 1.0, 0.01)

data.df <- lapply(1:k, function(i) {
    prob <- function(x) {
        p_base <- inv.logit(discriminations[i] * (x - difficulties[i]))
        return (chance + (1 - chance) * p_base)
    }
    data[,i] <<- sapply(prob(theta), function(x) { rbern(1, x) })

    subdf <- data.frame(sapply(xs, function(x) { prob(x) }))
    names(subdf) <- paste(sprintf('%.5f', difficulties[i]), sep='')
    subdf
}) %>% bind_cols()

data.df$x <- xs
data.df <- melt(data.df, id.vars='x', variable.name='difficulty')

## Sets ggplot2 and mcmc_intervals fonts to sans
set_font_size <- function(g, font_size) {
    g + theme(axis.text=element_text(family='sans', size=font_size),
              axis.title=element_text(family='sans', size=font_size),
              strip.text=element_text(family='sans', size=font_size),
              plot.title=element_text(family='sans', size=font_size))
}

set_font_size_no_xticks <- function(g, font_size) {
    g + theme(axis.text.x=element_blank(), axis.ticks.x=element_blank(),
              axis.text=element_text(family='sans', size=font_size),
              axis.title=element_text(family='sans', size=font_size),
              plot.title=element_text(family='sans', size=font_size))
}

png(filename='itr_probability.png', width=2000, height=800)
g <- ggplot(data.df, aes(x=x, y=value))
g <- g + geom_line(aes(colour=difficulty), size=2)
g <- g + theme(legend.position='none')
g <- g + labs(title='Each line corresponds to one question')
g <- g + xlab('Ability')
g <- g + ylab('Probability to choose correct answers')
g <- set_font_size(g, 32)
plot(g)
dev.off()

options(mc.cores = parallel::detectCores())
rstan_options(auto_write = TRUE)
Sys.setenv(LOCAL_CPPFLAGS = '-march=native')

## Estimates with Stan
input_data <- list(N=n, K=k, Y=data, chance=chance)
start_time <- proc.time()
fit.stan <- stan(file='item_response_theory.stan', data=input_data,
                 iter=stan_iter+stan_warmup, warmup=stan_warmup, chains=n_stan_chains, seed=common_seed)
elapsed_time <- proc.time() - start_time

fit.stan.array <- as.array(fit.stan)
fit.stan.names <- unlist(dimnames(fit.stan.array))
if (n_stan_chains == 1) {
    fit.stan.means <- get_posterior_mean(fit.stan)
} else {
    fit.stan.means <- get_posterior_mean(fit.stan)[,n_stan_chains + 1]
}
df_fit <- as.data.frame(fit.stan.means)

font_size <- 12
set_font <- function(g) {
    g + theme(axis.text=element_text(family='sans', size=font_size),
              axis.title=element_text(family='sans', size=font_size),
              strip.text=element_text(family='sans', size=font_size),
              plot.title=element_text(family='sans', size=font_size))
}

fit.ggs <- ggs(fit.stan)
png(filename='irt_all_difficulty_ggs.png', width=800, height=480)
g1 <- set_font(ggs_Rhat(fit.ggs, family="theta") + guides(color=guide_legend(override.aes = list(size=4))))
g2 <- set_font(ggs_histogram(fit.ggs, family="theta"))
g3 <- set_font(ggs_traceplot(fit.ggs, family="theta"))
grid.arrange(g1, g2, g3, ncol=3, nrow=1)
dev.off()

## Draws a chart of difficulties of questions
if (n > 6) {
    png(filename='irt_all_difficulty_hist.png', width=1200, height=800)
    stan_hist(fit.stan, pars=c('difficulty'), nrow=4, ncol=6, fill='slateblue')
    dev.off()
}

draw_charts <- function(actual, param_name, display_name, ylab_text) {
    param_name_regex <- paste('^', param_name, sep='')
    estimated <- df_fit[grepl(param_name_regex, rownames(df_fit)),]
    names(estimated) <- 'estimated'
    df_chart <- data.frame(estimated=estimated, actual=actual)
    df_chart$index <- 1:NROW(df_chart)
    df_chart$diff <- df_chart$estimated - df_chart$actual

    filename <- paste('itr_', display_name, '_hist.png', sep='')
    title <- paste('Difference between estimated and actual', display_name, sep=' ')
    png(filename=filename, width=800, height=800)
    g <- ggplot(df_chart)
    g <- g + geom_histogram(aes(x=diff))
    g <- g + labs(title=title)
    g <- g + xlab('Difference')
    g <- g + ylab('Frequency')
    g <- set_font_size(g, 24)
    plot(g)
    dev.off()

    filename <- paste('itr_', display_name, '_plot.png', sep='')
    title <- paste('Estimated and actual', display_name, sep=' ')
    png(filename=filename, width=1600, height=800)
    g <- ggplot(df_chart)
    g <- g + geom_point(aes(x=index, y=estimated), color='orchid', size=7)
    g <- g + geom_line(aes(x=index, y=actual), color='black', size=2)
    g <- g + labs(title=title)
    g <- g + xlab('Index')
    g <- g + ylab(ylab_text)
    g <- set_font_size(g, 32)
    plot(g)
    dev.off()

    filename <- paste('itr_', display_name, '_interval.png', sep='')
    png(filename=filename, width=1000, height=1000)
    g <- mcmc_intervals(fit.stan.array, rev(fit.stan.names[grepl(param_name_regex, fit.stan.names)]))
    g <- g + coord_flip()
    g <- g + labs(title=ylab_text)
    g <- set_font_size_no_xticks(g, 32)
    plot(g)
    dev.off()
}

draw_charts(difficulties, 'difficulty', 'difficulty', 'Difficulty')
draw_charts(theta, 'theta', 'ability', 'Ability')

## Print summary
summary(fit.stan)
print(fit.stan, pars=c('discrimination', 'difficulty'))
print(elapsed_time)
