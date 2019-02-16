library(ggplot2)
library(reshape2)

for (size in seq(1, 9, 2)) {
    for (prob in seq(0.1, 0.9, 0.2)) {
        for (k in seq(2, 5)) {
            n <- 10000
            y_all <- rnbinom(n, size=size, prob=prob)
            y_sum <- rowSums(replicate(k, rnbinom(n, size=size/k, prob=prob)))

            table_all <- tabulate(y_all)
            table_sum <- tabulate(y_sum)
            max_count <- max(length(table_all), length(table_sum))
            table_all <- append(table_all, rep(0, max_count - length(table_all)))
            table_sum <- append(table_sum, rep(0, max_count - length(table_sum)))

            df <- data.frame(all=table_all, sum=table_sum)
            df$x <- 1:NROW(df)
            df <- melt(df, c('x'))
            g <- ggplot(df, aes(x=x, y=value, color=variable))
            g <- g + geom_line(position = 'identity', size=1.5, alpha=0.6)
            plot(g)
        }
    }
}
