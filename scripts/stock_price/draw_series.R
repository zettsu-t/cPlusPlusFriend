library(ggplot2)
library(RColorBrewer)
library(scales)
library(plyr)
library(dplyr)

n_series <- 12
n_trial <- 4
n_epoch <- 1000

df <- lapply(1:n_series, function(i) {
    lapply(1:n_trial, function(trial) {
        ## Random walk
        sigma <- i
        ys <- abs(cumsum(rnorm(n_epoch, 0, sigma)))
        data.frame(series=rep(i, n_epoch), trial=rep(trial, n_epoch), sigma=rep(sigma, n_epoch), x=1:n_epoch, y=ys)
    }) %>% ldply(data.frame)
}) %>% ldply(data.frame)

draw_chart <- function(cols, bg_col, filename) {
    png(filename=filename, width=1024, height=768)
    g <- ggplot()
    for (df.sub in split(df, list(df$series, df$trial))) {
        col <- gradient_n_pal(cols)(df.sub$series / n_series)
        g <- g + geom_line(data=df.sub, aes(x=x, y=y), colour=col, alpha=0.6)
    }
    g <- g + theme(panel.background = element_rect(fill=bg_col))
    g <- g + labs(x='Epoch', y='Abs(Value)')
    plot(g)
    dev.off()
}

## Cited from https://sites.google.com/site/seascapemodelling/home/r-code/customising-plots/continous-colour-brewer-palettes
bluecols <- brewer.pal(9, 'Blues')
newcol <- colorRampPalette(bluecols)
cols <- newcol(n_series)
draw_chart(cols, "lightgray", "blue.png")

cols <- brewer.pal(12, 'Paired')
draw_chart(cols, "lightgray", "paired.png")

cols <- dichromat_pal("BrowntoBlue.12")(n_series)
draw_chart(cols, "black", "brown_to_blue.png")
