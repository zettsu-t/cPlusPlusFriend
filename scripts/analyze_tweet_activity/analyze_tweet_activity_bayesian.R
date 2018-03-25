## Analyze tweet impressions by posted timestamps

library(coda)
library(data.table)
library(rstan)
library(ggplot2)

infilename <- 'data/converted2.csv'
all_png_filename <- 'data/converted2_all.png'
png_basename <- 'data/converted2_'
png_extension <- '.png'

make_column_name <- function(prefix, index) {
    paste(prefix, '_', toString(index), sep='')
}

tweets <- data.table(read.csv(file=file.path(infilename),
                              header=TRUE, fileEncoding='utf-8',
                              stringsAsFactors=FALSE))

# Extract time indexes
unique_time_indexes <- unique(tweets$time_index)
time_indexes <- sort(unique_time_indexes[unique_time_indexes > 0])

# Posted at 07:00
tweets_base <- tweets[time_index == 0]

all_data <- NULL
diff_column_names <- NULL

for(i in time_indexes) {
    # Posted at 19:00
    tweets_alt <- tweets[time_index == i]

    input_data <- list(N_base=nrow(tweets_base), N_alt=nrow(tweets_alt),
                       Y_base=log(tweets_base$impressions), Y_alt=log(tweets_alt$impressions))
    fit <- stan(file='analyze_tweet_activity_bayesian.stan', data=input_data, seed=123)
    fit.coda <- mcmc.list(lapply(1:ncol(fit),function(x) mcmc(as.array(fit)[,x,])))

    pngfilename <- paste(png_basename, toString(i), png_extension, sep='')
    png(filename=pngfilename, width=1200, height=800)
    plot(fit.coda)
    dev.off()

    extracted_data <- as.data.frame(rstan::extract(fit))
    for (column_name in names(extracted_data)) {
        names(extracted_data)[names(extracted_data) == column_name] <- make_column_name(column_name, i)
    }
    diff_column_name <- make_column_name('difference', i)

    if (is.null(all_data)) {
       all_data <- extracted_data
       diff_column_names <- c(diff_column_name)
    } else {
       all_data <- cbind(all_data, extracted_data)
       diff_column_names <- cbind(diff_column_names, diff_column_name)
    }
}

all_diffs <- all_data[, diff_column_names]
g <- ggplot(all_diffs)
g <- g + stat_density(aes(x=difference_1, colour='11:00'), position='identity', geom='line', size=1.25)
g <- g + stat_density(aes(x=difference_2, colour='15:00'), position='identity', geom='line', size=1.25)
g <- g + stat_density(aes(x=difference_3, colour='19:00'), position='identity', geom='line', size=1.25)
g <- g + stat_density(aes(x=difference_4, colour='23:00'), position='identity', geom='line', size=1.25)
g <- g + labs(x='difference(log(tweet impressions)')
g <- g + theme(legend.position='top')
g <- g + labs(colour='Posted time (base 7:00)')
g <- g + scale_color_manual(values=c('firebrick', 'goldenrod3', 'dodgerblue4', 'black'))
g <- g + geom_vline(xintercept=0.0, size=2, linetype='dashed', color='black')
png(filename=all_png_filename, width=600, height=400)
plot(g)
dev.off()
