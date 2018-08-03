library(ggplot2)

## x : annual interest rate [%]
## let r be (100+x) / x
## r^n = 2 -> n*log(r) = log(2) -> n = log(2)/log(r)
years_to_double <- function(x) {
    log(2) / log((100.0 + x) / 100.0)
}

years_to_double_approximation <- function(x) {
    72.0 / x
}
xs <- seq(0.5, 10.0, by=0.25)

png(filename='out/interest.png', width=600, height=600)

df_actual <- data.frame(interest=xs, year=years_to_double(xs))
df_actual$Function <- 'Actual'
df_approx <- data.frame(interest=xs, year=years_to_double_approximation(xs))
df_approx$Function <- 'Approximation'
df <- rbind(df_actual, df_approx)

g <- ggplot(df, aes(x=interest, y=year, color=Function))
g <- g + geom_line(aes(linetype=Function), lwd=2)
g <- g + scale_linetype_manual(values=c('solid', 'dashed'))
g <- g + scale_color_manual(values=c('cyan','orchid'))
g <- g + labs(x='Annual interest rate [%]', y='Years to double')
g <- g + theme(legend.position = c(0.8, 0.8),
               legend.text=element_text(size=20), legend.title=element_text(size=20),
               axis.text=element_text(size=20), axis.title=element_text(size=20))
plot(g)

dev.off()
