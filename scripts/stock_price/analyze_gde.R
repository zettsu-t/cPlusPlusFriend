library(ggplot2)
library(tseries)
library(forecast)
library(plotrix)

## Reads the Gross Domestic Expenditure (GDE) historical data
## Data source
## http://www.esri.cao.go.jp/jp/sna/data/data_list/sokuhou/files/2005/qe052_2/gdemenujb.html
## http://www.esri.cao.go.jp/jp/sna/data/data_list/sokuhou/files/2005/qe052_2/__icsFiles/afieldfile/2012/02/29/gaku_smg0522_1.csv
data_description <- 'GDE'
default_infilename <- 'incoming/gde.csv'
default_out_basename <- 'out/gde'

args <- commandArgs(trailingOnly=TRUE)
in_filename <- ifelse(length(args) >= 1, args[1], default_infilename)
out_basename <- ifelse(length(args) >= 2, args[2], default_out_basename)
out_chart_filename <- paste(out_basename, '', '_chart.png', sep='')
out_diff_filename <- paste(out_basename, '', '_diff.png', sep='')
out_acf_filename <- paste(out_basename, '', '_acf.png', sep='')

## Read a CSV file (redundant lines are removed)
df <- read.csv(in_filename, header=F)

to_num <- function(s) {
    as.numeric(gsub(',', '', s))
}

## Differences of log(rates)
gdes <- to_num(df[,2])
df$log_gdes <- log(gdes)

png(filename=out_chart_filename, width=800, height=400)
g <- ggplot(df, aes(x=seq(1:nrow(df)), y=log_gdes))
g <- g + geom_line(lwd=2)
g <- g + scale_x_continuous(breaks = seq(1, nrow(df), 4))
g <- g + ggtitle('Gross Domestic Expenditure')
g <- g + xlab('quarter year') + ylab('log(GDE)')
g <- g + theme(plot.title = element_text(hjust = 0.5),
               panel.grid.major.x = element_line(size=1.5, color='orchid', linetype='dashed'),
               panel.grid.major.y = element_blank())
plot(g)
dev.off()
lag_colors <- c('black', 'violetred4', 'violetred2', 'navy')

png(filename=out_acf_filename, width=800, height=400)
par(mfrow=c(1,2))
acf(df$log_gdes, main=data_description, lwd=3)
pacf(df$log_gdes, main=data_description, lwd=3)
dev.off()

df_diff <- NULL
n_lag <- 4
for(lag in seq(1:n_lag)) {
    df_diff_part <- df
    df_diff_part$log_diff_gdes <- c(diff(df$log_gdes, lag), rep(NA, lag))
    df_diff_part$lag <- as.character(lag)
    df_diff_part <- df_diff_part[1:(nrow(df)-lag),]
    df_diff_part$term <- seq(1:nrow(df_diff_part))
    if (is.null(df_diff)) {
        df_diff <- df_diff_part
    } else {
       df_diff <- rbind(df_diff, df_diff_part)
    }
}

png(filename=out_diff_filename, width=800, height=400)
g <- ggplot(df_diff, aes(x=term, y=log_diff_gdes, color=lag))
g <- g + geom_line(aes(linetype=lag), lwd=2)
g <- g + scale_linetype_manual(values=c('solid', 'dashed', 'dashed', 'solid'))
g <- g + scale_color_manual(values=lag_colors)
g <- g + labs(x='quarter year', y='diff(log(GDE)) with n-lags')
g <- g + theme(legend.position = c(0.95, 0.85),
               legend.text=element_text(size=20), legend.title=element_text(size=20),
               axis.text=element_text(size=20), axis.title=element_text(size=20))
plot(g)
dev.off()

Box.test(df$log_gdes, lag=1)
Box.test(df$log_gdes, lag=4)
Box.test(df$log_gdes, lag=8)
