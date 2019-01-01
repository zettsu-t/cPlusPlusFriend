library(ggplot2)
library(plyr)
library(dplyr)

check_poisson <- function(mu, prob) {
##  mu <- size * (1 - prob) / prob
    size <- mu * prob / (1 - prob)
    x_max <- max(qnbinom(1e-3, size, prob, lower.tail=FALSE), qpois(1e-3, mu, lower.tail=FALSE)) + 1
    xs <- 0:x_max

    value_actual <- dnbinom(xs, size, prob)
    value_expected <- dpois(xs, mu)
    value_actual <- c(value_actual, 1.0 - sum(value_actual))
    value_expected <- c(value_expected, 1.0 - sum(value_expected))

    ## degree of freedom = length(xs) + 1 (others) - 2 = x_max + 2 - 2
    k <- x_max
    residuals <- sum(((value_actual - value_expected)^2) / value_expected)
    list(size=size, pvalue=pchisq(residuals, k, lower.tail=FALSE))
}

mu_set <- seq(from=0.01, to=75, by=0.005)
prob_set <- seq(from=0.3, to=0.7, by=0.002)
df <- lapply(mu_set, function (mu) {
    result_set <- sapply(prob_set, function(prob) { check_poisson(mu, prob) })
    size_set <- unlist(result_set['size',])
    pvalue_set <- unlist(result_set['pvalue',])
    data.frame(mu=mu, prob=prob_set, size=size_set, pvalue=pvalue_set)
}) %>% ldply(data.frame)

set_text_size <- function(g) {
    font_size <- 16
    g + theme(axis.text=element_text(family='sans', size=font_size),
              axis.title=element_text(family='sans', size=font_size),
              strip.text=element_text(family='sans', size=font_size),
              plot.title=element_text(family='sans', size=font_size))
}

png(filename='mu_prob.png', width=800, height=600)
g <- ggplot(df, aes(x=mu, y=prob))
g <- g + geom_tile(aes(fill=pvalue)) + scale_fill_gradient(low="white", high="royalblue4")
g <- set_text_size(g)
plot(g)
dev.off()

png(filename='size_prob.png', width=800, height=600)
## df <- df %>% dplyr::arrange(size, prob)
g <- ggplot(df, aes(x=size, y=prob))
g <- g + geom_point(aes(color=pvalue)) + scale_color_gradient(low="darkorchid2", high="royalblue4")
g <- set_text_size(g)
plot(g)
dev.off()
