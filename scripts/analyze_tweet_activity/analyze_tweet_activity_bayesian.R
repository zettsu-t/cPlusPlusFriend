## Analyze tweet impressions by posted timestamps
## and write charts for differences of tweet impressions
## Usage:
## $ Rscript analyze_tweet_activity_bayesian.R [input-csv-filename [output-file-basename]]

library(coda)
library(data.table)
library(rstan)
library(ggplot2)

default_infilename <- 'data/converted2.csv'
default_out_basename <- 'data/converted2'
png_extension <- '.png'

args <- commandArgs(trailingOnly=TRUE)
infilename <- ifelse(length(args) >= 1, args[1], default_infilename)
out_basename <- ifelse(length(args) >= 2, args[2], default_out_basename)
png_basename <- out_basename
all_png_filename <- paste(out_basename, '', '_all.png', sep='')

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
    # Posted at after 07:00
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
g <- g + theme(text = element_text(size=32))
g <- g + theme(legend.text=element_text(size=rel(1.0)))
g <- g + theme(axis.text.x=element_text(size=rel(1.5)))
g <- g + theme(plot.margin = unit(c(2, 2, 2, 2), "lines"))
g <- g + stat_density(aes(x=difference_1, colour='11:00'), position='identity', geom='line', size=1.5)
g <- g + stat_density(aes(x=difference_2, colour='15:00'), position='identity', geom='line', size=1.5)
g <- g + stat_density(aes(x=difference_3, colour='19:00'), position='identity', geom='line', size=1.5)
g <- g + stat_density(aes(x=difference_4, colour='23:00'), position='identity', geom='line', size=1.5)
g <- g + labs(title='Posterior distributions for differences in tweet impressions',
              x='difference(log(tweet impressions))')
g <- g + theme(legend.position='top')
g <- g + labs(colour='Posted time (base 7:00)')
g <- g + scale_color_manual(values=c('firebrick', 'goldenrod3', 'dodgerblue4', 'black'))
g <- g + geom_vline(xintercept=0.0, size=2, linetype='dashed', color='black')
png(filename=all_png_filename, width=1200, height=800)
plot(g)
dev.off()
