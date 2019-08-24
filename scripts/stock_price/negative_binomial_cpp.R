library(ggplot2)
library(reshape2)
library(plyr)
library(dplyr)

draw_chart <- function(filename) {
    df <- read.csv(filename)
    names(df)[names(df)=='alpha'] <- 'size'
    df <- df[, !(names(df) %in% c('beta'))]

    dfs <- df %>% dplyr::group_by(size, prob) %>%
        mutate(expected=dnbinom(x, size=size, prob=prob)) %>%
        mutate(pois=dpois(x, lambda=size*(1-prob)/prob))

    ## Print to check mean(dnbinom) is equal to mean(possion)
    dfs.summary <-dfs %>%
        do(mean_dbinom=mean(.$x * .$expected), mean_possion=mean(.$x * .$pois)) %>%
        summarise(mean_dbinom=mean_dbinom, mean_possion=mean_possion, diff=mean_dbinom - mean_possion)
    print(dfs.summary)
    dfs <- melt(dfs, id.vars=c('x', 'size', 'prob'))

    outfilename <- gsub('\\.csv', '.png', filename)
    png(filename=outfilename, width=800, height=480)
    g <- ggplot(dfs, aes(x=x, y=value, group=variable, color=variable))
    g <- g + geom_line()
    g <- g + scale_linetype_manual(values=c('solid', 'solid', 'dashed'))
    g <- g + scale_color_manual(labels=c('C++ Random', 'R dnbinom', 'Poisson'), values=c('navy', 'cornflowerblue', 'orchid'))
    g <- g + facet_grid(prob~size, scales='free', labeller=labeller(size=label_both, prob=label_both))
    g <- g + xlab('Count')
    g <- g + ylab('Density')
    g <- g + theme(legend.title=element_blank())
    plot(g)
    dev.off()
}

draw_random_nb <- function() {
    png(filename='negative_binomial_cpp_random.png', width=800, height=480)
    n <- 10000
    size <- 0.40
    prob <- 0.30
    df <- data.frame(value=rnbinom(n=n, size=size, prob=prob))
    g <- ggplot(df, aes(x=value))
    g <- g + geom_histogram(bins=max(df$value+1), color='black', fill='royalblue')
    g <- g + ggtitle(sprintf("Negative binomial distribution (n=%d, size=%.2f, prob=%.2f): random numbers", n, size, prob))
    plot(g)
    dev.off()
}

draw_random_nb()
files <- c('nbinom100.csv', 'nbinom1k.csv', 'nbinom10k.csv', 'nbinom100k.csv')
sapply(files, draw_chart)
