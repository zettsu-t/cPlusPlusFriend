library(tidyverse)
library(ggpattern)

df <- tibble(
  label = c("A", "B", "C", "D"),
  value = c(40, 35, 30, 25),
  colour = c("cornflowerblue", "royalblue1", "royalblue3", "royalblue4"),
  density = c(0.4, 0.3, 0.2, 0)
)

g <- ggplot(df)
g <- g + geom_bar_pattern(
  aes(x = label, y = value, fill = colour, pattern_density = density),
  pattern = "circle", pattern_fill = "gray80", colour = "black", stat = "identity")
g <- g + scale_fill_manual(values = df$colour)
g <- g + ggtitle("A sample of ggpattern::geom_bar_pattern")
g <- g + theme(legend.position = "none", plot.title=element_text(size = 12))
plot(g)
ggsave("chart_texture.png", g, width = 4, height = 3)
