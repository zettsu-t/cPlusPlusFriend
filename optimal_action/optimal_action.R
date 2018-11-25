library(ggplot2)
library(reshape2)

df.all <- read.csv('optimal_action.csv')
df.all$epoch <- 1:nrow(df.all)

initial_length <- 250
long_length <- 100000

setup_chart <- function(g) {
    g <- g + scale_color_manual(values=c('royalblue', 'violet', 'black'))
    g <- g + xlab('Epoch')
    g <- g + ylab('Input of Softmax')
    g <- g + theme(axis.text=element_text(family='sans', size=20),
                   axis.title=element_text(family='sans', size=20),
                   strip.text=element_text(family='sans', size=20),
                   plot.title=element_text(family='sans', size=20))
    g <- g + theme(legend.position = c(0.1, 0.3),
                   plot.title=element_text(size=24),
                   legend.title=element_text(size=20),
                   legend.text=element_text(size=20))
    g
}

df <- melt(df.all[1:initial_length,], id='epoch')
names(df) <- c('epoch', 'target', 'value')
png(filename='initial.png', width=1200, height=400)
g <- ggplot(df)
g <- g + geom_line(aes(x=epoch, y=value, color=target))
g <- setup_chart(g)
plot(g)
dev.off()

df <- melt(df.all[1:long_length,], id='epoch')
names(df) <- c('epoch', 'target', 'value')
png(filename='log_linear.png', width=1200, height=400)
g <- ggplot(df)
g <- g + geom_line(aes(x=epoch, y=value, color=target))
g <- g + scale_x_log10()
g <- setup_chart(g)
plot(g)
dev.off()
