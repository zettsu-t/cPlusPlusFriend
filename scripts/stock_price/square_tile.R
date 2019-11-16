library(dplyr)
library(ggplot2)

xs <- seq(-2.5, 2.5, 0.05)
ys <- seq(-2.5, 2.5, 0.05)

df <- lapply(ys, function(y) {
    vs <- sapply(xs, function(x) {
        dnorm(abs(x) + abs(y))
    })
    tibble(x=xs, y=y, value=vs)
}) %>% dplyr::bind_rows()

make_image <- function(df, title) {
    g <- ggplot(df, aes(x=x, y=y, fill=value))
    g <- g + geom_tile(show.legend = FALSE)
    g <- g + scale_fill_gradient(low='white', high='royalblue4')
    g <- g + ggtitle(paste('dnorm(abs(x) + abs(y))', title))
    g <- g + labs(x='x', y='y')
    g <- g + coord_equal()
    g <- g + theme(aspect.ratio=1,
                   legend.position='none',
                   plot.background=element_blank(),
                   panel.background=element_blank())
    g <- g + scale_x_continuous(expand=c(0,0))
    g <- g + scale_y_continuous(expand=c(0,0))
    g
}

ggsave('dnorm_abs_ggsave.png', plot=make_image(df, ': ggsave'), dpi=100)
png(filename='dnorm_abs_png.png', width=1040, height=700)
plot(make_image(df, ': png_plot'))
dev.off()
