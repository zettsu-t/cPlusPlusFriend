library(rstan)

## Reads the Gross Domestic Expenditure (GDE) historical data
## Data source
## http://www.esri.cao.go.jp/jp/sna/data/data_list/sokuhou/files/2005/qe052_2/gdemenujb.html
## http://www.esri.cao.go.jp/jp/sna/data/data_list/sokuhou/files/2005/qe052_2/__icsFiles/afieldfile/2012/02/29/gaku_smg0522_1.csv
data_description <- 'GDE'
default_infilename <- 'incoming/gde.csv'

args <- commandArgs(trailingOnly=TRUE)
in_filename <- ifelse(length(args) >= 1, args[1], default_infilename)

## Read a CSV file (redundant lines are removed)
df <- read.csv(in_filename, header=F)

to_num <- function(s) {
    as.numeric(gsub(",", "", s))
}

gdes <- to_num(df[,2])
df$log_gdes <- log(gdes)
input_data <- list(T=4, N=NROW(df), X=df$log_gdes)
fit <- stan(file='analyze_gde.stan', data=input_data, chains=3, seed=123)
summary(fit)
