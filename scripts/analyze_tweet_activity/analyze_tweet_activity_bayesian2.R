## Analyze tweet impressions by posted timestamps

library(coda)
library(data.table)
library(rstan)
library(ggplot2)
library(ggmcmc)

infilename <- 'data/converted2.csv'
mcmc_png_filename <- 'data/converted2_mcmc.pdf'
all_png_filename <- 'data/converted2_all.png'
png_basename <- 'data/converted2_'
png_extension <- '.png'

make_column_name <- function(prefix, delimiter, index) {
    paste(prefix, delimiter, toString(index), sep='')
}

tweets <- data.table(read.csv(file=file.path(infilename),
                              header=TRUE, fileEncoding='utf-8',
                              stringsAsFactors=FALSE))

# Posted at 07:00
tweets_base <- tweets[time_index == 0]

# Extract time indexes
unique_time_indexes <- unique(tweets$time_index)
time_indexes <- sort(unique_time_indexes[unique_time_indexes > 0])

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

ggmcmc(ggs(fit), file=mcmc_png_filename)
extracted_data <- as.data.frame(rstan::extract(fit))

g <- ggplot(extracted_data)
g <- g + theme(text = element_text(size=32))
g <- g + theme(legend.text=element_text(size=rel(1.0)))
g <- g + theme(axis.text.x=element_text(size=rel(1.5)))
g <- g + theme(plot.margin = unit(c(2, 2, 2, 2), "lines"))
g <- g + stat_density(aes(x=difference.1, colour='11:00'), position='identity', geom='line', size=1.5)
g <- g + stat_density(aes(x=difference.2, colour='15:00'), position='identity', geom='line', size=1.5)
g <- g + stat_density(aes(x=difference.3, colour='19:00'), position='identity', geom='line', size=1.5)
g <- g + stat_density(aes(x=difference.4, colour='23:00'), position='identity', geom='line', size=1.5)
g <- g + labs(title='Posterior distributions for differences in tweet impressions',
              x='difference(log(tweet impressions))')
g <- g + theme(legend.position='top')
g <- g + labs(colour='Posted time (base 7:00)')
g <- g + scale_color_manual(values=c('firebrick', 'goldenrod3', 'dodgerblue4', 'black'))
g <- g + geom_vline(xintercept=0.0, size=2, linetype='dashed', color='black')
png(filename=all_png_filename, width=1200, height=800)
plot(g)
dev.off()
