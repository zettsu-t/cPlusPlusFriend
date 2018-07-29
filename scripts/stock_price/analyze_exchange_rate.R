library(ggplot2)
library(tseries)
library(forecast)
library(plotrix)

## Reads the real effective exchange rate (JPY) historical data
## Data source
## https://www.stat-search.boj.or.jp/
data_description <- 'Real effective exchange rate (JPY)'
default_infilename <- 'incoming/ExchangeRate.csv'
default_out_basename <- 'out/rate'

args <- commandArgs(trailingOnly=TRUE)
in_filename <- ifelse(length(args) >= 1, args[1], default_infilename)
out_basename <- ifelse(length(args) >= 2, args[2], default_out_basename)
out_chart_filename <- paste(out_basename, '', '_chart.png', sep='')
out_acf_filename <- paste(out_basename, '', '_acf.png', sep='')

## Read a CSV file
df_base <- df <- read.csv(in_filename, skip=1, header=T)

## Differences of log(rates)
rate_log_diff <- diff(log(df$Rate))

add_day <- function(s) {
    paste(s, "/01", sep="")
}

df$Month <- as.Date(add_day(df$Month), format="%Y/%m/%d")
df$log_diff[2:nrow(df)] <- rate_log_diff

## Modeling
model <- auto.arima(rate_log_diff, ic="aic", trace=F, stepwise=F, approximation=F, seasonal=F,
                    max.p=6, max.q=0, max.order=12)
print(model)

## ADF testing
result.adf <- adf.test(rate_log_diff)
print(result.adf)

df_diffs <- df[-c(1),]
month_seq <- 1:nrow(df_diffs)

png(filename=out_chart_filename, width=800, height=600)
twoord.plot(month_seq, df_diffs$Rate,
            month_seq, df_diffs$log_diff,
            xlab=data_description, ylab='Rate', rylab='diff(log(Rate))',
            type=c('l', 'l'), lcol=c('blue', 'blue'), rcol=c('darkorchid2', 'darkorchid2'),
            xticklab=c(df_diffs$Month))
dev.off()

png(filename=out_acf_filename, width=800, height=400)
par(mfrow=c(1,2))
acf(df$Rate, main=data_description)
pacf(df$Rate, main=data_description)
dev.off()
