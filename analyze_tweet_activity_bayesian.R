## Analyze tweet impressions by posted timestamps

library(data.table)
library(rstan)
library(ggplot2)

infilename <- 'data/converted2.csv'
d <- data.table(read.csv(file=file.path(infilename),
                         header=TRUE, fileEncoding='utf-8',
                         stringsAsFactors=FALSE))

# Posted at 07:00
d_morning <- d[time_index == 0]
# Posted at 19:00
d_night <- d[time_index == 3]

data <- list(N_morning=nrow(d_morning), N_night=nrow(d_night), Y_morning=log(d_morning$impressions), Y_night=log(d_night$impressions))
fit <- stan(file='analyze_tweet_activity_bayesian.stan', data=data, seed=123)
