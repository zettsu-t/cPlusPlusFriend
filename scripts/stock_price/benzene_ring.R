## Drawing the benzene ring symbol
library(tidyverse)
library(ggforce)

unit_length <- 0.3
height_for_base <- sqrt(3) / 2.0
height <- unit_length
width <- height * height_for_base
half_height <- height / 2.0

df_benzene_hexagon <- tibble::tibble(
  x = c(0, -width, -width, 0, width, width),
  y = c(height, half_height, -half_height, -height, -half_height, half_height)
)

x_to_set <- c(tail(df_benzene_hexagon$x, -1), head(df_benzene_hexagon$x, 1))
y_to_set <- c(tail(df_benzene_hexagon$y, -1), head(df_benzene_hexagon$y, 1))
df_benzene_hexagon$x_to <- x_to_set
df_benzene_hexagon$y_to <- y_to_set

df_benzene_ring <- tibble::tibble(x0 = 0.0, y0 = 0.0, r = width * 0.8)
df_methyl_group <- tibble::tibble(x = 0, y = height, x_to = 0, y_to = height * 25 / 14, x_pos = 0.065)
df_methyl_group$name <- as.character(expression("CH"[3]))

df_rect <- tibble::tibble(
  xmin = min(df_benzene_hexagon$x),
  xmax = max(df_benzene_hexagon$x),
  ymin = min(df_benzene_hexagon$y),
  ymax = max(df_benzene_hexagon$y) + 0.25
)

g <- ggplot()
g <- g + geom_rect(aes(xmin = xmin, xmax = xmax, ymin = ymin, ymax = ymax), fill = "white", data = df_rect)
g <- g + geom_link(aes(x = x, y = y, xend = x_to, yend = y_to),
  color = "black", size = 3, lineend = "round", data = df_benzene_hexagon
)
g <- g + geom_segment(aes(x = x, y = y, xend = x_to, yend = y_to),
  color = "black", size = 3, data = df_methyl_group
)
g <- g + geom_circle(aes(x0 = x0, y0 = y0, r = r),
  color = "black", size = 3, data = df_benzene_ring
)
g <- g + geom_label(aes(x = x_pos, y = y_to, label = name),
  label.size = NA, fill = "white", cex = 15, data = df_methyl_group, parse = TRUE
)
g <- g + coord_fixed()
g <- g + theme_light()
g <- g + theme(
  panel.border = element_blank(), axis.ticks = element_blank(),
  panel.grid.major = element_blank(), panel.grid.minor = element_blank(),
  axis.text = element_blank(), axis.title = element_blank()
)
plot(g)
ggsave(filename = "toluene.png", plot = g, dpi = 96)
