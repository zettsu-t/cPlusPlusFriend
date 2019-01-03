library(ggplot2)
library(reshape2)

check_cpp_rnbinom <- function(expected_size, expected_prob, filename) {
    v <- read.csv(file=filename, header = FALSE)
    actual_size <- v[1,1]
    actual_prob <- v[2,1]
    value_set <- v[-c(1:2),]
    xs <- 0:(NROW(value_set)-1)
    actual <- value_set / sum(value_set)
    expected <- dnbinom(xs, size=expected_size, prob=expected_prob)

    df <- data.frame(x=xs, actual=actual, expected=expected)
    df.melt <- melt(df, id.vars='x')
    g <- ggplot(df.melt, aes(x=x, y=value, fill=variable, colour=variable))
    g <- g + geom_line()
    g <- g + xlab('Count')
    g <- g + ylab('Density')
    plot(g)
}

check_cpp_rnbinom(1.0,  0.25, "nbinom_1.csv")
check_cpp_rnbinom(1.0,  0.50, "nbinom_2.csv")
check_cpp_rnbinom(1.0,  0.75, "nbinom_3.csv")
check_cpp_rnbinom(2.0,  0.25, "nbinom_4.csv")
check_cpp_rnbinom(3.0,  0.40, "nbinom_5.csv")
check_cpp_rnbinom(5.0,  0.50, "nbinom_6.csv")
check_cpp_rnbinom(10.0, 0.50, "nbinom_7.csv")
check_cpp_rnbinom(15.0, 0.25, "nbinom_8.csv")
check_cpp_rnbinom(15.0, 0.75, "nbinom_9.csv")
