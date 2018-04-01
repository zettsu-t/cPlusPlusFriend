## Analyze tweet impressions by posted timestamps
## and write charts for differences of tweet impressions
## Usage:
## $ Rscript analyze_tweet_activity_bayesian2.R [input-csv-filename [output-file-basename]]

library(coda)
library(data.table)
library(rstan)
library(ggplot2)
library(ggmcmc)

default_infilename <- 'data/converted2.csv'
default_out_basename <- 'out/converted2'
png_extension <- '.png'

args <- commandArgs(trailingOnly=TRUE)
infilename <- ifelse(length(args) >= 1, args[1], default_infilename)
out_basename <- ifelse(length(args) >= 2, args[2], default_out_basename)
all_png_filename <- paste(out_basename, '', '_all.png', sep='')
mcmc_filename <- paste(out_basename, '', '_mcmc.pdf', sep='')

tweets <- data.table(read.csv(file=file.path(infilename),
                              header=TRUE, fileEncoding='utf-8',
                              stringsAsFactors=FALSE))

# Posted at 07:00
tweets_base <- tweets[time_index == 0]

# Extract time indexes
unique_time_indexes <- unique(tweets$time_index)
time_indexes <- sort(unique_time_indexes[unique_time_indexes > 0])
# hours betwwen tweets
hour_span <- 16 %/% length(time_indexes)

all_data <- NULL
diff_column_names <- NULL
tweets_alt <- tweets[time_index > 0]
input_data <- list(N_base=nrow(tweets_base), Y_base=log(tweets_base$impressions),
                   K=length(time_indexes), N=nrow(tweets_alt),
                   TID=tweets_alt$time_index, Y=log(tweets_alt$impressions))

fit <- stan(file='analyze_tweet_activity_bayesian2.stan', data=input_data, seed=123)
fit.coda <- mcmc.list(lapply(1:ncol(fit),function(x) mcmc(as.array(fit)[,x,])))
# Print R-hats
print(fit)

extracted_data <- as.data.frame(rstan::extract(fit))

g <- ggplot(extracted_data)
g <- g + theme(text = element_text(size=32))
g <- g + theme(legend.text=element_text(size=rel(1.0)))
g <- g + theme(axis.text.x=element_text(size=rel(1.5)))
g <- g + theme(plot.margin = unit(c(2, 2, 2, 2), "lines"))

if (hour_span == 4) {
    g <- g + stat_density(aes(x=difference.1, colour='11:00'), position='identity', geom='line', size=1.5)
    g <- g + stat_density(aes(x=difference.2, colour='15:00'), position='identity', geom='line', size=1.5)
    g <- g + stat_density(aes(x=difference.3, colour='19:00'), position='identity', geom='line', size=1.5)
    g <- g + stat_density(aes(x=difference.4, colour='23:00'), position='identity', geom='line', size=1.5)
    g <- g + scale_color_manual(values=c('firebrick', 'goldenrod3', 'dodgerblue4', 'black'))
}

if (hour_span == 2) {
    g <- g + stat_density(aes(x=difference.1, colour='09:00'), position='identity', geom='line', size=1.5, linetype='dashed')
    g <- g + stat_density(aes(x=difference.2, colour='11:00'), position='identity', geom='line', size=1.5)
    g <- g + stat_density(aes(x=difference.3, colour='13:00'), position='identity', geom='line', size=1.5, linetype='dashed')
    g <- g + stat_density(aes(x=difference.4, colour='15:00'), position='identity', geom='line', size=1.5)
    g <- g + stat_density(aes(x=difference.5, colour='17:00'), position='identity', geom='line', size=1.5, linetype='dashed')
    g <- g + stat_density(aes(x=difference.6, colour='19:00'), position='identity', geom='line', size=1.5)
    g <- g + stat_density(aes(x=difference.7, colour='21:00'), position='identity', geom='line', size=1.5, linetype='dashed')
    g <- g + stat_density(aes(x=difference.8, colour='23:00'), position='identity', geom='line', size=1.5)
    g <- g + scale_color_manual(values=c('firebrick2', 'firebrick', 'goldenrod2', 'goldenrod4',
                                         'dodgerblue1', 'dodgerblue3', 'blue4', 'black'))
}

g <- g + labs(title='Posterior distributions for differences in tweet impressions',
              x='difference(log(tweet impressions))')
g <- g + theme(legend.position='top')
g <- g + labs(colour='Posted time (base 7:00)')
g <- g + geom_vline(xintercept=0.0, size=2, linetype='dashed', color='black')
png(filename=all_png_filename, width=1200, height=800)
plot(g)
dev.off()

ggmcmc(ggs(fit), file=mcmc_filename)
