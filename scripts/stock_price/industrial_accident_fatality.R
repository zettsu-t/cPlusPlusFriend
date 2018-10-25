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
df <- read.csv('industrial_accident_fatality.csv')
years_tick_width <- 10
years_min <- floor(min(df$year) / years_tick_width) * years_tick_width
years_max <- floor(max(df$year) / years_tick_width) * years_tick_width
year_breaks <- seq(years_min, years_max, years_tick_width)

png(filename='changepoints_base.png', width=1200, height=600)
g <- ggplot(df)
g <- g + geom_line(aes(x=year, y=fatality), size=2, color='black', linetype='solid')
g <- g + stat_smooth(aes(x=year, y=fatality), method = "lm", se = FALSE, colour = "orange", linetype='dashed', size = 2)
plot(g)
dev.off()


## Detects change points with the changepoint package
points <- cpt.meanvar(df$fatality)@cpts
years <- df$year[points[-length(points)]]

## Estimates with JAGS
input_data <- list(T=nrow(df), Y=df$fatality)
fit.jags <- jags.model(file='industrial_accident_fatality_jags.txt', data=input_data,
                       n.chains=3, n.adapt=20000)
update(fit.jags, n.iter=1000)
result.jags <- jags.samples(fit.jags,
                            variable.names = c('mu_e', 'mu_l', 'tau_e', 'tau_l', 'cp'),
                            n.iter=20000)
summary(result.jags)
jags_cp <- round(mean(result.jags$cp))
year_jags <- df$year[jags_cp]

## Estimates with Stan
input_data <- list(T=nrow(df), X=df$year, Y=df$fatality)
fit.stan <- stan(file='industrial_accident_fatality.stan', data=input_data,
                 iter=10000, warmup=5000, chains=1, seed=common_seed)
summary(fit.stan)
stan_tau <- round(mean(extract(fit.stan)$tau))
year_stan <- df$year[stan_tau]


## Draw the estimations on the observed data
draw_estimation <- function(filename, base_df, mu_e, sigma_e, mu_l, sigma_l, trend_l, tau) {
    df <- base_df
    ribbon_width <- 1.96

    ys_e <- rep(mu_e, tau)
    ys_min_e <- ys_e - sigma_e * ribbon_width
    ys_max_e <- ys_e + sigma_e * ribbon_width
    n_l <- nrow(df) - tau
    ys_l <- seq(mu_l, mu_l + trend_l * (n_l - 1), length.out=n_l)
    ys_min_l <- ys_l - sigma_l * ribbon_width
    ys_max_l <- ys_l + sigma_l * ribbon_width

    df$estimated_y <- c(ys_e, ys_l)
    df$estimated_ymin <- c(ys_min_e, ys_min_l)
    df$estimated_ymax <- c(ys_max_e, ys_max_l)

    png(filename=filename, width=1200, height=600)
    g <- ggplot(df)
    g <- g + geom_line(aes(x=year, y=fatality), size=2, color='black', linetype='solid')
    g <- g + geom_line(aes(x=year, y=estimated_y), size=2, color='red', linetype='solid')
    g <- g + geom_ribbon(aes(x=year, ymin=estimated_ymin, ymax=estimated_ymax), fill = "grey70", alpha=0.5)
    plot(g)
    dev.off()
}

draw_estimation('changepoints_stan.png', df,
                get_posterior_mean(fit.stan)['mu_e',],
                get_posterior_mean(fit.stan)['sigma_e',],
                get_posterior_mean(fit.stan)['mu_l',],
                get_posterior_mean(fit.stan)['sigma_l',],
                get_posterior_mean(fit.stan)['trend_l',],
                stan_tau)

estemated_sigma_e <- 1.0 / sqrt(result.jags$tau_e[1])
estemated_sigma_l <- 1.0 / sqrt(result.jags$tau_l[1])

draw_estimation('changepoints_jags.png', df,
                result.jags$mu_e[1],
                estemated_sigma_e,
                result.jags$mu_l[1],
                estemated_sigma_l,
                0.0, jags_cp)


## Draw the detected change points
png(filename='changepoints_timeseries.png', width=1200, height=600)
g <- ggplot(df)
g <- g + geom_line(aes(x=year, y=fatality), size=2, color='black', linetype='solid')

color_str <- 'slateblue'
texts <- sapply(years, function(x) {paste(x, '(CP)', sep=' ')})
g <- g + geom_text(aes(x=years, y=0), label=texts, colour=color_str, hjust=1.2, vjust=0.1, size=8)
g <- g + geom_vline(xintercept=years, colour=color_str, linetype='solid', size=2)

color_str <- 'brown'
text <- paste(year_jags, '(JAGS)', sep=' ')
g <- g + geom_vline(xintercept=year_jags, colour=color_str, linetype='solid', size=2)
g <- g + geom_text(aes(x=year_jags, y=0), label=text, colour=color_str, hjust=1.2, vjust=-3.1, size=8)

color_str <- 'violet'
text <- paste(year_stan, '(Stan)', sep=' ')
g <- g + geom_vline(xintercept=year_stan, colour=color_str, linetype='solid', size=2)
g <- g + geom_text(aes(x=year_stan, y=0), label=text, colour=color_str, hjust=1.2, vjust=-1.6, size=8)

g <- g + scale_x_continuous(breaks=year_breaks)
g <- g + theme(axis.text=element_text(size=24), axis.title=element_text(size=24), plot.title=element_text(size=24))
g <- g + labs(title='Industrial Accident Fatalities (yearly)')
plot(g)
dev.off()


## Draw the posterior distribution
## https://rpubs.com/uri-sy/iwanami_ds1
df_param <- ggs(fit.stan)
patterns <- c('^mu_(e|l)', '^sigma_', '^(trend_|tau)')
postfixes <- c('mu', 'sigma', 'tau')
for (i in 1:length(patterns)) {
    fit <- df_param %>% dplyr::filter(grepl(patterns[i], Parameter))
    filename = paste('changepoint_', postfixes[i], '.png', sep='')
    png(filename=filename, width=800, height=1200)
    g <- ggs_histogram(fit)
    g <- g + theme(axis.text=element_text(size=24), axis.title=element_text(size=24), strip.text=element_text(size=24))
    plot(g)
    dev.off()
}
