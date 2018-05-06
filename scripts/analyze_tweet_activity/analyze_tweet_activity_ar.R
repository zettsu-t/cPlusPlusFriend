library(ggplot2)
library(tseries)
library(forecast)

default_infilename <- 'data/converted3.csv'
default_out_basename <- 'out/converted3'
args <- commandArgs(trailingOnly=TRUE)
in_filename <- ifelse(length(args) >= 1, args[1], default_infilename)
out_basename <- ifelse(length(args) >= 2, args[2], default_out_basename)
impression_png_filename <- paste(out_basename, '', '_impressions.png', sep='')
resid_png_filename <- paste(out_basename, '', '_residential.png', sep='')

## Read tweet impressions
df_base <- read.csv(in_filename)

## Excludes irregular posts if they exist
## df1 <- df_base[-(first:last),]
df1 <- df_base

## Excludes outliers manually
df2 <- df1[df1$impressions > 50,]
df.full <- df2[df2$impressions < 10000,]

## It is known that tweet impressions are under a log-normal distribution
df.full$log_impressions <- log(df.full$impressions)
df.autopost <- df.full[df.full$autopost == "True",]

analyze_ar <- function(df, out_impression_png_filename, out_resid_png_filename) {
    ## Tweets are ordered from newer to older.
    ## When users view a tweet, they may view older tweets of its author
    ## and are not likely to wait newer tweets of the author.
    g <- ggplot(df, aes(x=index, y=log_impressions))
    g <- g + geom_line()
    png(filename=out_impression_png_filename, width=1600, height=800)
    plot(g)
    dev.off()

    # ADF testing
    result.adf <- adf.test(df$log_impressions)
    print(result.adf)

    # Modeling
    m <- auto.arima(df$log_impressions, ic="aic", trace=F, stepwise=F, approximation=F, seasonal=F, max.p=6, max.q=0, max.order=12)

    ## Check residential. Based on
    ## http://nakhirot.hatenablog.com/entry/20130703/1372788018
    n_lag <- 15
    pvalue <- numeric(n_lag)
    for(i in 1:15) {
        pvalue[i] <- Box.test(m$resid, lag=i, type="Ljung-Box")$p.value
    }

    pvalue.df <- as.data.frame(pvalue)
    pvalue.df$lag <- 1:nrow(pvalue.df)
    g <- ggplot(pvalue.df, aes(x=lag, y=pvalue))
    g <- g + geom_line()
    png(filename=out_resid_png_filename, width=600, height=400)
    plot(g)
    dev.off()

    m
}

model <- analyze_ar(df.full, impression_png_filename, resid_png_filename)
summary(model)

## Confirm a model manually
## arima(df.full$log_impressions, order=c(4,0,0))

