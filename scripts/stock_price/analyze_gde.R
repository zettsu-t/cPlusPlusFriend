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
out_acf_filename <- paste(out_basename, '', '_acf.png', sep='')

## Read a CSV file (redundant lines are removed)
df <- read.csv(in_filename, header=F)

to_num <- function(s) {
    as.numeric(gsub(",", "", s))
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

png(filename=out_acf_filename, width=800, height=400)
par(mfrow=c(1,2))
acf(df$log_gdes, main=data_description, lwd=3)
pacf(df$log_gdes, main=data_description, lwd=3)
dev.off()

Box.test(df$log_gdes)
Box.test(diff(df$log_gdes))
