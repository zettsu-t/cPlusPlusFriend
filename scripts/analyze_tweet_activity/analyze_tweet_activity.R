## Analyze tweet impressions
## usage : Rscript analyze_tweet_activity.R infile outfile
## infile : Name of exported tweet analytics CSV file (input)
## outfile : Name of PNG image file (output)

library(Cairo)
library(data.table)
library(fitdistrplus)

default_infilename <- 'data/in.csv'   # input data filename
default_outfilename <- 'data/out.png' # output image filename
low_impression_threshold  <- 60   # 1/3 of followers
high_impression_threshold <- 403  # exp(6)

## Assume tweets are posted by a bot when they are posted at
## even-hour, 00-05 minute UTC.
is_bot <- function(time_str) {
    ifelse((grep(' \\d\\[02468]:\\d[0-5] ', time_str) == 0), FALSE , TRUE)
}

## Draws P-P plot and histogram
## X-axis: impression (linear), Y-axis: frequency of tweets (linear or log)
draw_pp_hist <- function(data, model, title, xlabel, show_estimation) {
    ylabel <- 'Frequency of tweets'
    ## Estimates a distribution under the 'model'
    fd <- fitdist(data, model)
    ## P-P plot
    ppcomp(fd, legendtext='tweet', fitcol='violetred')

    if (show_estimation == TRUE) {
        hist(data, xlab=xlabel, ylab=ylabel, main=title, prob=TRUE, xaxt='n', yaxt='n')
        axis(side=1, labels=FALSE)
        # Overwrites an estimatsion under the 'model'.
        # The first parameter for curve must have name 'x'.
        x <- data
        curve(dnorm(x, mean=fd$estimate[1], sd=fd$estimate[2]),
              xlab=xlabel, col='royalblue', lwd=2, add=TRUE)
    } else {
        hist(data, xlab=xlabel, ylab=ylabel, main=title, prob=TRUE, xaxt='n', yaxt='n')
        axis(side=1, labels=FALSE)
    }
}

## Draws P-P plot and histogram (tweet impressions are plotted on a log scale)
draw_pp_loghist <- function(data, title, xlabel) {
    hist.data <- hist(data, breaks=10, xlab='ln(Impressions)', ylab='Frequency of tweets',
                     main=title, prob=TRUE, xaxt='n', yaxt='n')

    ## Draws a chart in log-log scales to confirm whether data are
    ## distributed under the power law.
    hist_log.data <- hist(data, breaks=10, plot=FALSE)
    hist_log.data$counts <- log(hist_log.data$counts + 1)
    plot(hist_log.data, # xaxt='n', yaxt='n',
         main=title, xlab='ln(Impressions)', ylab='ln(Frequency of tweets)')

    ## Overwrites a regression line
    m <- lm(hist_log.data$counts ~ hist_log.data$mids)
    abline(m, col='royalblue', lwd=2)
}

## Draws all charts
draw_all_pp_hist <- function(impressions, low_impressions, high_impressions, fontsize) {
    par(mfcol=c(2, 5), ps=fontsize)

    draw_pp_hist(impressions, 'norm', 'Impressions', 'Impressions', FALSE)
    draw_pp_hist(low_impressions, 'norm', 'Except high Impressions', 'Impressions', TRUE)

    draw_pp_hist(log(impressions), 'norm', 'Impressions', 'ln(Impressions)', FALSE)
    draw_pp_hist(log(low_impressions), 'norm', 'Except high impressions', 'ln(Impressions)', TRUE)

    draw_pp_loghist(log(high_impressions), 'High impressions', 'ln(Impressions)')
}

## Input and output filenames
args <- commandArgs(trailingOnly=TRUE)
infilename <- ifelse(length(args) >= 1, args[1], default_infilename)
outfilename <- ifelse(length(args) >= 2, args[2], default_outfilename)

## Loads a tweet activity CSV file (its titles must be swapped to English)
raw_data <- data.table(read.csv(file=file.path(infilename),
                                header=TRUE, fileEncoding='utf-8',
                                stringsAsFactors=FALSE))

## Excludes N/A data
data <- raw_data[complete.cases(raw_data),]

## Sets whether each tweets are possibly posted by the bot
data$bot <- lapply(data$time, is_bot)
## Excludes manual posts
data$bot <- lapply(data$bot, function(x) { ifelse(x == TRUE, TRUE, NA) })
data <- data[!is.na(data$bot)]

## Cut tweets with low impressions that suggest they are missed in observations
impressions <- data$impression[data$impression >= low_impression_threshold]
## Separate low impressions from high impressions which have different distributions
low_impressions <- impressions[impressions < high_impression_threshold]
high_impressions <- impressions[impressions >= high_impression_threshold]

## Draws on screen and writes to a PNG file
draw_all_pp_hist(impressions, low_impressions, high_impressions, 16)
png(filename=outfilename, width=1600, height=800)
draw_all_pp_hist(impressions, low_impressions, high_impressions, 16)
dev.off()
