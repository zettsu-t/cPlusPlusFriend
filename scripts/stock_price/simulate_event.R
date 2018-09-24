library(ggplot2)
library(raster)
library(rstan)

## Fix the random seed to reproduce results
common_seed <- 123
seed <- common_seed

## Size of input data
n_weeks <- 52
day_of_weeks <- rep(1:7, n_weeks)
n_days <- length(day_of_weeks)

## Hyper parameters
## Commutors ~ normal(mu_{weekday|holiday}, sigma_{weekday|holiday})
## Event visitors ~ log_normal(mu_event, sigma_event)
##                ~ mu_event * exp(normal(0, sigma_event))
hyper_mu_weekday <- 1.0      ## Baseline : linear
hyper_sigma_weekday <- 0.03  ## linear and >0
hyper_mu_holiday <- 0.6      ## linear
hyper_sigma_holiday <- 0.10  ## linear and >0
hyper_mu_event <- 0.05       ## log normal
hyper_sigma_event <- 0.25    ## log normal and >0
hyper_event_prob <- 0.6      ## E[event=1]

## Generates commutors
rand_commutor_of_day <- function(day_of_week) {
    is_holiday <- ifelse(day_of_week > 5, 1, 0)
    ifelse(is_holiday > 0,
           rnorm(1, hyper_mu_holiday, hyper_sigma_holiday),
           rnorm(1, hyper_mu_weekday, hyper_sigma_weekday))
}

commutors <- sapply(day_of_weeks, rand_commutor_of_day)
commutors <- clamp(commutors, 0, max(commutors))

## Generate event visitors
rand_event_visitor <- function(is_open) {
    ifelse(is_open == 0, 0, hyper_mu_event * head(rlnorm(1, 0, hyper_sigma_event)))
}

event_opens <- sample(x=c(0,1), size=n_days, replace=TRUE, prob=c(1.0-hyper_event_prob, hyper_event_prob))
event_visitors <- sapply(event_opens, rand_event_visitor)

## Chops redundant data
event_opens <- head(event_opens, n_days)
event_visitors <- head(event_visitors, n_days)
event_visitors <- clamp(event_visitors, 0, max(event_visitors))

visitors <- commutors + event_visitors
df <- data.frame(day=seq(1,n_days), day_of_week=day_of_weeks, event=event_opens,
                 commutor=commutors, event_visitor=event_visitors, visitor=visitors)
write.csv(df, 'visitors.csv')
g <- ggplot(df)
g <- g + geom_line(aes(x=day, y=visitor), size = 0.5, color="black", linetype="solid")
plot(g)

input_data <- list(T=length(visitors), W=n_weeks, Y=visitors, E=event_opens)
fit.stan <- stan(file='simulate_event.stan', data=input_data, iter=5000, warmup=2500, chains=1, seed=common_seed)
summary(fit.stan)
