library(changepoint)
library(timeSeries)
library(ggplot2)
library(rjags)
library(raster)
library(rstan)

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

## Detects change points with the changepoint package
points <- cpt.meanvar(df$fatality)@cpts
years <- df$year[points[-length(points)]]

input_data <- list(T=nrow(df), Y=df$fatality)
fit.jags <- jags.model(file='industrial_accident_fatality_jags.txt', data=input_data,
                       n.chains=3, n.adapt=10000)
update(fit.jags, n.iter=5000)
result.jags <- jags.samples(fit.jags,
                            variable.names = c('mu_e', 'mu_l', 'sigma_e', 'sigma_l', 'trend_l', 'tau'),
                            n.iter=10000)
summary(result.jags)
jags_tau <- round(mean(result.jags$tau))
year_jags <- df$year[jags_tau]

input_data <- list(T=nrow(df), X=df$year, Y=df$fatality)
fit.stan <- stan(file='industrial_accident_fatality.stan', data=input_data,
                 iter=10000, warmup=5000, chains=1, seed=common_seed)
summary(fit.stan)
stan_tau <- round(mean(extract(fit.stan)$tau))
year_stan <- df$year[stan_tau]

png(filename='changepoints.png', width=800, height=600)
g <- ggplot(df)
g <- g + geom_line(aes(x=year, y=fatality), size=2, color='black', linetype='solid')

color_str <- 'slateblue'
texts <- sapply(years, function(x) {paste(x, '(CP)', sep=' ')})
g <- g + geom_text(aes(x=years, y=0), label=texts, colour=color_str, hjust=1.2, vjust=0.1, size=5)
g <- g + geom_vline(xintercept=years, colour=color_str, linetype='solid', size=2)

color_str <- 'orange'
text <- paste(year_jags, '(JAGS)', sep=' ')
g <- g + geom_vline(xintercept=year_jags, colour=color_str, linetype='solid', size=2)
g <- g + geom_text(aes(x=year_jags, y=0), label=text, colour=color_str, hjust=1.2, vjust=-3.1, size=5)

color_str <- 'violet'
text <- paste(year_stan, '(Stan)', sep=' ')
g <- g + geom_vline(xintercept=year_stan, colour=color_str, linetype='solid', size=2)
g <- g + geom_text(aes(x=year_stan, y=0), label=text, colour=color_str, hjust=1.2, vjust=-1.6, size=5)

g <- g + scale_x_continuous(breaks=year_breaks)
g <- g + theme(axis.text=element_text(size=12), axis.title=element_text(size=16))
g <- g + labs(title='Industrial Accident Fatalities (yearly)')
plot(g)
dev.off()
