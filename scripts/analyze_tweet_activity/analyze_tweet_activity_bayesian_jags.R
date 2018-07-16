## Compare tweet impressions 7am and 7pm

library(coda)
library(data.table)
library(rjags)
library(ggplot2)

default_infilename <- 'data/converted2.csv'
default_out_basename <- 'out/converted2'
png_extension <- '.png'

args <- commandArgs(trailingOnly=TRUE)
infilename <- ifelse(length(args) >= 1, args[1], default_infilename)
out_basename <- ifelse(length(args) >= 2, args[2], default_out_basename)
png_basename <- out_basename
all_png_filename <- paste(out_basename, '', '_jags.png', sep='')

tweets <- data.table(read.csv(file=file.path(infilename),
                              header=TRUE, fileEncoding='utf-8',
                              stringsAsFactors=FALSE))

# Posted at 7am and 7pm
tweets_7am <- log(tweets[time_index == 0]$impressions)
tweets_7pm <- log(tweets[time_index == 3]$impressions)
input_data <- list(Y_7am=tweets_7am, Y_7pm=tweets_7pm, N_7am=length(tweets_7am), N_7pm=length(tweets_7pm))
fit <- jags.model(file='analyze_tweet_activity_bayesian_jags.txt', data=input_data, n.chains=3, n.adapt=500)
update(fit, n.iter=5000)
result <- coda.samples(fit, variable.names = c('mu_7am', 'mu_7pm', 'diff'), n.iter=20000)
summary(result)
png(filename=all_png_filename, width=800, height=800)
plot(result)
dev.off()
