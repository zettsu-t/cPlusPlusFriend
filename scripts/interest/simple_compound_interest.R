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

## Accumulate under the simple interest
accumulate_simple_interest <- function(odds_list) {
    accum_sum <- 0.0
    count <- 0.0
    values <- c()
    for (odds in odds_list) {
        count <- count + 1.0
        accum_sum <- accum_sum + odds
        values <- cbind(values, accum_sum / count)
    }
    values
}

## Accumulate under the compound interest
accumulate_compound_interest <- function(odds_list) {
    accum_product <- 1.0
    count <- 0.0
    values <- c()
    for (odds in odds_list) {
        count <- count + 1.0
        accum_product <- accum_product * odds
        values <- cbind(values, accum_product)
    }
    values
}

## Simulate
n_epoch_simple <- 1000
n_epoch_compound <- 10
n_amount_breaks <- c(0,1,1.75,2.5,5,10)
n_epoch_breaks <- c(0,2,4,6,8,10)
n_epoch <- max(n_epoch_simple, n_epoch_compound)
n_trial <- 100

for (i in 1:n_trial) {
    odds <- roll_dice(n_epoch)
    v.simple <- accumulate_simple_interest(odds[1:n_epoch_simple])
    v.compound <- accumulate_compound_interest(odds[1:n_epoch_compound])
    df.simple.i <- data.frame('run'=as.factor(rep(i,n_epoch_simple)),'epoch'=1:n_epoch_simple,'amount'=as.vector(v.simple))
    df.compound.i <- data.frame('run'=as.factor(rep(i,n_epoch_compound)),'epoch'=1:n_epoch_compound,'amount'=as.vector(v.compound))
    if (i == 1) {
        df.simple <- df.simple.i
        df.compound <- df.compound.i
    } else {
        df.simple <- rbind(df.simple, df.simple.i)
        df.compound <- rbind(df.compound, df.compound.i)
    }
}

## Draw charts
g <- ggplot(df.simple, aes(x=epoch, y=amount, colour=run))
g <- g + geom_line(alpha=0.2)
g <- g + labs(title="Simple interest") + theme(legend.position="none")
g <- g + geom_hline(yintercept = 1) + scale_y_continuous(breaks=n_amount_breaks)
g <- g + geom_hline(yintercept = 1.75, linetype='dashed')
plot(g)
ggsave(filename='simple.png', g,width = 5, height = 4, dpi = 150, units = "in", device='png')

pd <- position_dodge(1.0)
g <- ggplot(df.compound, aes(x=epoch, y=amount, colour=run))
g <- g + geom_line(position = pd, alpha=0.3) + scale_x_continuous(breaks=n_epoch_breaks)
g <- g + geom_hline(yintercept = 1) + scale_y_log10()
g <- g + labs(title="Compound interest") + theme(legend.position="none")
plot(g)
ggsave(filename='compound.png', g,width = 5, height = 4, dpi = 150, units = "in", device='png')
