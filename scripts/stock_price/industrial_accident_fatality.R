library(changepoint)
library(dplyr)
library(ggmcmc)
library(ggplot2)
library(raster)
library(rjags)
library(rstan)
library(timeSeries)

## Example for change point detection
## Industrial accident fatalities in Japan since 1958
## Data source are
## https://www.mhlw.go.jp/stf/houdou/0000165073.html
## http://www.aichi-c.ed.jp/digitalcontents/Web_koug/zenpan/anzen/anzen_top.htm

common_seed <- 123

## Set numbers of iterations in testing
n_jags_chains <- 3
n_stan_chains <- 3
jags_adapt <- 1000
jags_burn <- 10000
jags_iter <- 20000
stan_warmup <- 500
stan_iter <- 1000

## Reads data
df <- read.csv('industrial_accident_fatality.csv')
years_tick_width <- 10
years_min <- floor(min(df$year) / years_tick_width) * years_tick_width
years_max <- floor(max(df$year) / years_tick_width) * years_tick_width
year_breaks <- seq(years_min, years_max, years_tick_width)

## Plots the input data and adds a linear regression
set_font_size <- function(g, font_size) {
    g + theme(axis.text=element_text(family='sans', size=font_size),
              axis.title=element_text(family='sans', size=font_size),
              strip.text=element_text(family='sans', size=font_size),
              plot.title=element_text(family='sans', size=font_size))
}

png(filename='changepoints_base.png', width=2000, height=1000)
g <- ggplot(df)
g <- g + geom_line(aes(x=year, y=fatality), size=3, color='black', linetype='solid')
g <- g + stat_smooth(aes(x=year, y=fatality), method='lm', se=FALSE,
                     colour='darkgoldenrod2', linetype='dashed', size=4)
g <- set_font_size(g, 32)
plot(g)
dev.off()


## Detects change points with the changepoint package
points <- cpt.meanvar(df$fatality)@cpts
years <- df$year[points[-length(points)]]

## Estimates with JAGS
input_data <- list(T=NROW(df), Y=df$fatality)
fit.jags <- jags.model(file='industrial_accident_fatality_jags.txt', data=input_data,
                       n.chains=n_jags_chains, n.adapt=jags_adapt)
update(fit.jags, n.iter=jags_burn)
result.jags <- jags.samples(fit.jags,
                            variable.names = c('mu_e', 'mu_l', 'tau_e', 'tau_l', 'cp'),
                            n.iter=jags_iter)
summary(result.jags)
jags_cp <- round(mean(result.jags$cp))
year_jags <- df$year[jags_cp]

## Estimates with Stan
input_data <- list(T=NROW(df), X=df$year, Y=df$fatality)
fit.stan <- stan(file='industrial_accident_fatality.stan', data=input_data,
                 iter=stan_iter+stan_warmup, warmup=stan_warmup,
                 chains=n_stan_chains, seed=common_seed)
summary(fit.stan)
stan_tau <- base::round(mean(extract(fit.stan)$tau))
year_stan <- df$year[stan_tau]


## Draw the estimations on the observed data
draw_estimation <- function(filename, base_df, mu_e, sigma_e, mu_l, sigma_l, trend_l, tau) {
    df <- base_df
    ribbon_width <- 1.96

    ys_e <- rep(mu_e, tau)
    ys_min_e <- ys_e - sigma_e * ribbon_width
    ys_max_e <- ys_e + sigma_e * ribbon_width
    n_l <- NROW(df) - tau
    ys_l <- seq(mu_l, mu_l + trend_l * (n_l - 1), length.out=n_l)
    ys_min_l <- ys_l - sigma_l * ribbon_width
    ys_max_l <- ys_l + sigma_l * ribbon_width

    df$estimated_y <- c(ys_e, ys_l)
    df$estimated_ymin <- c(ys_min_e, ys_min_l)
    df$estimated_ymax <- c(ys_max_e, ys_max_l)

    png(filename=filename, width=2000, height=1000)
    g <- ggplot(df)
    g <- g + geom_line(aes(x=year, y=fatality), size=2, color='black', linetype='solid')
    g <- g + geom_line(aes(x=year, y=estimated_y), size=2, color='red', linetype='solid')
    g <- g + geom_ribbon(aes(x=year, ymin=estimated_ymin, ymax=estimated_ymax), fill = 'grey70', alpha=0.5)
    g <- g + labs(title='Industrial Accident Fatalities (yearly)')
    g <- set_font_size(g, 32)
    plot(g)
    dev.off()
}


estemated_sigma_e <- 1.0 / sqrt(result.jags$tau_e[1])
estemated_sigma_l <- 1.0 / sqrt(result.jags$tau_l[1])

draw_estimation('changepoints_jags.png', df,
                result.jags$mu_e[1],
                estemated_sigma_e,
                result.jags$mu_l[1],
                estemated_sigma_l,
                0.0, jags_cp)

## Use means of all chains if available
if (n_stan_chains == 1) {
    draw_estimation('changepoints_stan.png', df,
                    get_posterior_mean(fit.stan)['mu_e',],
                    get_posterior_mean(fit.stan)['sigma_e',],
                    get_posterior_mean(fit.stan)['mu_l',],
                    get_posterior_mean(fit.stan)['sigma_l',],
                    get_posterior_mean(fit.stan)['trend_l',],
                    stan_tau)
} else {
    index <- n_stan_chains + 1
    draw_estimation('changepoints_stan.png', df,
                    get_posterior_mean(fit.stan)['mu_e',][index],
                    get_posterior_mean(fit.stan)['sigma_e',][index],
                    get_posterior_mean(fit.stan)['mu_l',][index],
                    get_posterior_mean(fit.stan)['sigma_l',][index],
                    get_posterior_mean(fit.stan)['trend_l',][index],
                    stan_tau)
}

## Draw the detected change points
draw_changepoint <- function(g, years, text_str, color_str, vjust) {
    if (length(years) > 1) {
        text <- sapply(years, function(x) {paste(x, text_str, sep=' ')})
    } else {
        text <- paste(years, text_str, sep=' ')
    }
    g <- g + geom_vline(xintercept=years, colour=color_str, linetype='solid', size=2)
    g <- g + geom_text(aes(x=years, y=0), label=text, colour=color_str, hjust=1.2, vjust=vjust, size=14)
}

png(filename='changepoints_timeseries.png', width=2000, height=1000)
g <- ggplot(df)
g <- g + geom_line(aes(x=year, y=fatality), size=2, color='black', linetype='solid')
g <- draw_changepoint(g, years, '(CP)', 'slateblue', -0.1)
g <- draw_changepoint(g, year_jags, '(JAGS)', 'darkgoldenrod3', -3.1)
g <- draw_changepoint(g, year_stan, '(Stan)', 'darkmagenta', -1.6)
g <- g + scale_x_continuous(breaks=year_breaks)
g <- set_font_size(g, 32)
g <- g + labs(title='Industrial Accident Fatalities (yearly)')
plot(g)
dev.off()

## Draw the posterior distribution
## https://rpubs.com/uri-sy/iwanami_ds1
df_param <- ggs(fit.stan)
patterns <- c('^mu_(e|l)', '^sigma_', '^(trend_|tau)')
postfixes <- c('mu', 'sigma', 'tau')

sapply(1:length(patterns), function(i) {
    fit <- df_param %>% dplyr::filter(grepl(patterns[i], Parameter))
    filename <- paste('changepoint_', postfixes[i], '.png', sep='')
    png(filename=filename, width=1000, height=1500)
    g <- ggs_histogram(fit)
    g <- set_font_size(g, 32)
    plot(g)
    dev.off()
    TRUE
})
