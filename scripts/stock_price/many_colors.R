library(tidyverse)
library(ggplot2)
library(colorspace)
library(pals)

n <- 240
df <- tibble(x = 1, y = seq_len(n))
g <- ggplot(df)
g <- g + geom_tile(aes(x = x, y = y, fill = factor(y)))
## g <- g + scale_fill_manual(values = colorspace::diverge_hcl(n))
g <- g + scale_fill_manual(values = pals::brewer.blues(n))
g <- g + theme(aspect.ratio = 4,
               legend.position = "right",
               legend.text = element_text(size = 6),
               legend.title = element_blank(),
               axis.text = element_blank(),
               axis.title = element_blank(),
               strip.text = element_blank(),
               plot.title = element_blank())
plot(g)
