library(ggplot2)
library(ggthemes)

seed <- 123

## 1 for win and others for loss
get_odds <- function(n) {
    ifelse(n == 1, 10.0, 0.1)
}

## Roll a dice n times and return its odds
roll_dice <- function(n) {
    dices <- sample(1:6, n, replace=T)
    sapply(dices, get_odds)
}

## Kelly Criterion
## bet 1.0 and return 10.0 (p=1/6) or 0.1 (p=5/6)
## => keep 0.1 + bet 0.9 and return 9.9 (p=1/6) or 0 (p=5/6)
kelly_criterion <- (11.0 / 6.0 - 1.0) / (11.0 - 1.0)
ratio <- kelly_criterion * 0.9

## Accumulate under the compound interest
accumulate_compound_interest <- function(odds_list, ratio) {
    accum_product <- 1.0
    count <- 0.0
    values <- c()
    for (odds in odds_list) {
        count <- count + 1.0
        accum_product <- accum_product * ((1.0 - ratio) + ratio * odds)
        values <- cbind(values, accum_product)
    }
    values
}

## Simulate
n_epoch <- 100
n_trial <- 1000
n_amount_breaks <- c(0.1, 1.0, 10.0, 100.0, 1000.0, 10000.0)

for (i in 1:n_trial) {
    odds <- roll_dice(n_epoch)
    v <- accumulate_compound_interest(odds[1:n_epoch], ratio)
    df.i <- data.frame('run'=as.factor(rep(i,n_epoch)),'epoch'=1:n_epoch,'amount'=as.vector(v))
    if (i == 1) {
        df <- df.i
    } else {
        df <- rbind(df, df.i)
    }
}

pd <- position_dodge(0.9)
g <- ggplot(df, aes(x=epoch, y=amount, colour=run))
g <- g + geom_line(position = pd) + scale_y_log10(breaks=n_amount_breaks)
g <- g + labs(title="Compound interest") + theme(legend.position="none")
g <- g + geom_hline(yintercept = 1)
plot(g)
ggsave(filename='partial_reinvest.png', g,width = 6, height = 4, dpi = 150, units = "in", device='png')
