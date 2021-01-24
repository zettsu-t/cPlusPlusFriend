library(tidyverse)
library(egg)
library(extrafont)
library(magick)

draw_tile <- function(width, height, aspect_ratio, tile_color) {
    vs <- (1:(width * height)) / 100
    xs <- rep(1:width, times = 1, length.out = NA, each = height)
    ys <- rep(1:height, times = width, length.out = NA, each = 1)
    df <- tibble(x = xs, y = ys, value = vs)

    font_name <- "Segoe UI"
    g <- ggplot(df, aes(x = x, y = y))
    g <- g + geom_tile(aes(fill = value))
    g <- g + scale_fill_gradient(low = "black", high = tile_color)

    g <- g + if (is.na(aspect_ratio)) {
        theme(panel.background = element_blank(),
              text=element_text(family=font_name),
              axis.title = element_text(family=font_name, size=10),
              axis.text = element_text(family=font_name, size=8))
    } else {
        theme(aspect.ratio = aspect_ratio,
              panel.background = element_blank(),
              text=element_text(family=font_name),
              axis.title = element_text(family=font_name, size=10),
              axis.text = element_text(family=font_name, size=8))
    }

    g
}

draw_tiles <- function(width, height, aspect_ratio, tile_colors, png_filename) {
    g_list <- purrr::reduce(.x = seq_len(NROW(tile_colors)), .init = list(), function(acc, i) {
        c(acc, list(draw_tile(width = width, height = height,
                              aspect_ratio = aspect_ratio, tile_color = tile_colors[i])))
    })

    g <- egg::ggarrange(plots = g_list, nrow = 1, padding = unit(0, "line"),
                        plot.background = element_rect(fill = "#ffffff", color = "#ffffff"))
    ggsave(png_filename, g, dpi=96)
}

draw_trim_tiles <- function(width, height, aspect_ratio, tile_colors, png_filename, trimmed_filename) {
    draw_tiles(width = width, height = height, aspect_ratio = aspect_ratio,
               tile_colors = tile_colors, png_filename = png_filename)

    magick::image_read(png_filename) %>%
        magick::image_trim(fuzz = 10) %>%
        magick::image_repage() %>%
        magick::image_write(path = trimmed_filename, format = "png")
}

width <- 200
height <- 300
tile_colors <- c("royalblue", "orange", "gray70")

draw_tiles(width = width, height = height, aspect_ratio = NA, tile_colors = tile_colors,
           png_filename = "no_aspect_ratio.png")

draw_trim_tiles(width = width, height = height, aspect_ratio = height / width, tile_colors = tile_colors,
                png_filename = "concat.png", trimmed_filename = "trimmed.png")
