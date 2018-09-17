library(KFAS)
library(ggplot2)

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

## Read a CSV file (redundant lines are removed)
df <- read.csv(in_filename, header=F)

to_num <- function(s) {
    as.numeric(gsub(",", "", s))
}

gdes <- to_num(df[,2])
df$log_gdes <- log(gdes)
input_data <- list(T=4, N=nrow(df), X=df$log_gdes)

n_predict <- 4
n_train <- nrow(df) - n_predict

## Fit the GDE time series to a state space model
model <- SSModel(data = df[1:n_train,], distribution="gaussian", H = NA,
                 formula = log_gdes ~ SSMtrend(degree = 1, Q = NA)
                 + SSMseasonal(period = 4, sea.type = "dummy", Q = NA))
fit <- fitSSM(model, inits = c(1, 1, 1))

train_conf <- predict(fit$model, interval = "confidence", level = 0.95, type = "response")
predict_conf <- predict(fit$model, interval = "prediction", level = 0.95, n.ahead = n_predict)
df_fit <- rbind(data.frame(conf=train_conf), data.frame(conf=predict_conf))
df_fit$term <- 1:nrow(df_fit)
terms_predict <- seq(n_train,n_train+n_predict)

g <- ggplot()
g <- g + geom_line(aes(x=seq(1,nrow(df)), y=df$log_gdes), size = 0.5, color="black", linetype="solid")
g <- g + geom_line(aes(x=seq(1,n_train), y=df_fit$conf.fit[c(seq(1,n_train))]), size = 0.5, color="blue", alpha=.8, linetype="solid")
g <- g + geom_line(aes(x=terms_predict, y=df_fit$conf.fit[c(terms_predict)]), size = 0.5, color="blue", alpha=.8, linetype="dashed")
g <- g + geom_ribbon(aes(x=seq(1:nrow(df_fit)), ymin=df_fit$conf.lwr, ymax=df_fit$conf.upr), fill="blue", alpha=.3)
g <- g + ggtitle('GDE and prediction')
g <- g + theme(plot.title = element_text(hjust = 0.5))
g <- g + labs(x='quarter year', y='log(GDE)')

out_png_filename <- paste(out_basename, '', '_kfas.png', sep='')
png(filename=out_png_filename, width=600, height=400)
plot(g)
dev.off()
