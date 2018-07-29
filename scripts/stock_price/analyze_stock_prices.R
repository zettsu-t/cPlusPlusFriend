library(ggplot2)
library(tseries)
library(forecast)
library(plotrix)

## Reads TOPIX (or other) historical prices
## Data source
## https://quotes.wsj.com/index/JP/I0000/historical-prices
data_description <- 'TOPIX Index'
default_infilename <- 'incoming/HistoricalPrices.csv'
default_out_basename <- 'out/price'

args <- commandArgs(trailingOnly=TRUE)
in_filename <- ifelse(length(args) >= 1, args[1], default_infilename)
out_basename <- ifelse(length(args) >= 2, args[2], default_out_basename)
out_chart_filename <- paste(out_basename, '', '_chart.png', sep='')
out_acf_filename <- paste(out_basename, '', '_acf.png', sep='')

## Read a CSV file and reverses it
df_base <- df <- read.csv(in_filename)
df <- df_base[nrow(df_base):1,]

## Differences of log(prices)
price_log_diff <- diff(log(df$Close))
df$day <- as.Date(df$Date, format="%m/%d/%y")
df$log_diff[2:nrow(df)] <- price_log_diff

## Modeling
model <- auto.arima(price_log_diff, ic="aic", trace=F, stepwise=F, approximation=F, seasonal=F,
                    max.p=6, max.q=0, max.order=12)
print(model)

## ADF testing
result.adf <- adf.test(price_log_diff)
print(result.adf)

df_diffs <- df[-c(1),]
day_seq <- 1:nrow(df_diffs)

png(filename=out_chart_filename, width=600, height=600)
twoord.plot(day_seq, df_diffs$log_diff,
            day_seq, df_diffs$Close,
            xlab=data_description, ylab='diff(log(Price))', rylab='Price',
            type=c('l', 'l'), lwd=3, lcol=c('darkorchid2', 'darkorchid2'), rcol=c('blue', 'blue'),
            xticklab=c(df_diffs$day))
dev.off()

png(filename=out_acf_filename, width=800, height=400)
par(mfrow=c(1,2))
acf(df$Close, lwd=3)
pacf(df$Close, lwd=3)
dev.off()
