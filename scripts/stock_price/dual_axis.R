library(tidyverse)
library(ggplot2)
library(extrafont)

n <- 10000
va <- cumsum(rnorm(n = n))
vb <- cumsum(rnorm(n = n))
df <- tibble(index = seq_len(NROW(va)), a = va, b = vb) %>%
    tidyr::pivot_longer(cols = c("a", "b")) %>%
    dplyr::mutate(name = factor(name))

## Random walk
font_name <- 'Migu 1M'
png(filename='dual_scale.png', width=600, height=400)
g <- ggplot(df, aes(x = index, y = value, color = name))
g <- g + geom_line()
g <- g + scale_y_continuous(sec.axis = sec_axis(~ . * 10))
g <- g + scale_color_manual(values = c("orchid", "royalblue"))
g <- g + ggtitle("Random walk")
g <- g + xlab("index")
g <- g + ylab("value")
g <- g + theme(text=element_text(family=font_name),
               legend.position="none",
               axis.text.x=element_text(family=font_name, color='black', size=16),
               axis.text.y=element_text(family=font_name, color='black', size=16),
               axis.title.x=element_text(family=font_name, color='black', size=18),
               axis.title.y=element_text(family=font_name, color='black', size=18),
               plot.title=element_text(family=font_name, size=24))
plot(g)
dev.off()

va <- rnorm(n = n)
vb <- rnorm(n = n, mean = 0.2)
df <- tibble(index = seq_len(NROW(va)), a = va, b = vb) %>%
    tidyr::pivot_longer(cols = c("a", "b")) %>%
    dplyr::mutate(altname = stringr::str_c("別名_", name)) %>%
    dplyr::mutate(name = factor(name), altname = factor(altname))

## https://stackoverflow.com/questions/45361904/duplicating-and-modifying-discrete-axis-in-ggplot2
font_name <- 'Migu 1M'
png(filename='dual_category.png', width=600, height=400)
g <- ggplot(df, aes(y = value, x = as.numeric(name), color = name))
g <- g + geom_boxplot()
g <- g + scale_x_continuous(breaks = seq_len(NROW(df$name)),
                            labels = df$name,
                            sec.axis = sec_axis(~., breaks = seq_len(NROW(df$altname)),
                                                labels = df$altname))
g <- g + scale_color_manual(values = c("orchid", "royalblue"))
g <- g + ggtitle("rnorm")
g <- g + ylab("value")
g <- g + coord_flip()
g <- g + theme(text=element_text(family=font_name),
               legend.position="none",
               axis.text.x=element_text(family=font_name, color='black', size=18),
               axis.text.y=element_text(family=font_name, color='black', size=24),
               axis.title.x=element_text(family=font_name, color='black', size=24),
               axis.title.y=element_blank(),
               plot.title=element_text(family=font_name, size=24))
plot(g)
dev.off()
