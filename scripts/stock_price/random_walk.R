library(ggplot2)
library(Rmisc)

## Make a random walk chart
seed <- 1234
mu <- 0
sd <- 1
n <- 80

index <- seq(1:n)
white_noise <- rnorm(n, mu, sd)
random_walk <- cumsum(white_noise)
df <- data.frame(white_noise, random_walk)

png(filename='out/random_walk.png', width=1000, height=500)
g1 <- ggplot(df, aes(x=index, y=white_noise))
g1 <- g1 + geom_line(color='blue', lwd=2)
g1 <- g1 + ggtitle('White noise')
g1 <- g1 + theme(plot.title = element_text(hjust = 0.5))

g2 <- ggplot(df, aes(x=index, y=random_walk))
g2 <- g2 + geom_line(color='orchid', lwd=2)
g2 <- g2 + geom_line()
g2 <- g2 + ggtitle('Random walk')
g2 <- g2 + theme(plot.title = element_text(hjust = 0.5))
multiplot(g1, g2, cols=2)
dev.off()
