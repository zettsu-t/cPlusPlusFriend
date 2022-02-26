## Drawing the Barnsley fern
library(tidyverse)
library(abind)
library(assertthat)
library(Cairo)
library(extrafont)
library(raster)
library(zeallot)

## Based on
## https://jp.mathworks.com/matlabcentral/communitycontests/contests/4/entries/586
g_next_edges <- c(rep.int(x = 1:4, times = c(1, 85, 7, 7)))
invisible(assertthat::assert_that(NROW(g_next_edges) == 100))

g_transform_set <- array(data = c(
  0.0, 0.85, -0.15, 0.20,
  0.0, -0.04, 0.26, 0.23,
  0.0, 0.04, 0.28, -0.26,
  0.16, 0.85, 0.24, 0.22
), dim = c(4, 2, 2))

g_translate_set <- array(
  data = c(
    0.0, 0.0, 0.0, 0.0,
    0.0, 1.6, 1.6, 0.44
  ),
  dim = c(4, 1, 2)
)

g_font_name <- "Segoe UI"

select_next_edges <- function(n_edges) {
  max_num <- NROW(g_next_edges)
  indexes <- raster::clamp(
    x = ceiling(runif(n = n_edges, min = 0, max = max_num)),
    lower = 1, upper = max_num
  )
  g_next_edges[indexes]
}

## rows: x and y coordinates, cols = points
make_point_transform <- function(edges) {
  n_edges <- NROW(edges)
  transform_set <- array(data = 0.0, dim = c(n_edges, 2, 2))
  translate_set <- array(data = 0.0, dim = c(n_edges, 1, 2))

  purrr::map(seq_along(edges), function(i) {
    edge <- edges[i]
    transform_set[i, , ] <<- g_transform_set[edge, , , drop = FALSE]
    translate_set[i, , ] <<- g_translate_set[edge, , , drop = FALSE]
  })

  list(transform = transform_set, translate = translate_set)
}

transform_points <- function(points) {
  n_points <- NROW(points)
  next_edges <- select_next_edges(n_points)
  point_transform_set <- make_point_transform(next_edges)
  next_points <- array(data = 0.0, dim = c(n_points, 1, 2))

  purrr::walk(seq_len(n_points), function(i) {
    next_points[i, , ] <<- point_transform_set$transform[i, , ] %*% points[i, , , drop = FALSE]
  })
  next_points + point_transform_set$translate
}

make_fern <- function(n_iter) {
  initial_point <- array(data = c(0.0, 0.0), dim = c(1, 1, 2))
  purrr::reduce(.x = seq_len(n_iter), .init = list(points = initial_point), .f = function(acc, i) {
    new_points <- transform_points(acc$points)
    list(points = abind::abind(acc$points, new_points, along = 1))
  })
}

draw_fern <- function(fern, png_renderer, renderer_name, file_basename, pixel_size, int_coord) {
  xs <- fern$points[, , 1]
  ys <- fern$points[, , 2]
  n_points <- NROW(xs)
  suffix <- paste0("_", n_points)

  if (int_coord) {
    c(min_x, max_x) %<-% range(xs)
    c(min_y, max_y) %<-% range(ys)
    max_size <- max(max_x - min_x, max_y - min_y)
    xs <- floor(pixel_size * (xs - min_x) / max_size)
    ys <- floor(pixel_size * (ys - min_y) / max_size)
    suffix <- paste0(suffix, "i")
  }
  df <- tibble::tibble(x = xs, y = ys)

  start_time <- proc.time()
  g <- ggplot(df)
  g <- g + geom_point(aes(x = x, y = y), color = "mediumblue", size = 3, alpha = 0.2, shape = 18)
  g <- g + ggtitle(renderer_name)
  g <- g + theme_light()
  g <- g + theme(
    aspect.ratio = 1.0, panel.background = element_rect(fill = "wheat2"),
    panel.border = element_blank(), axis.ticks = element_blank(),
    panel.grid.major = element_blank(), panel.grid.minor = element_blank(),
    axis.text = element_blank(), axis.title = element_blank(),
    plot.title = element_text(family = g_font_name, size = 8)
  )

  full_filename <- paste0(file_basename, suffix, ".png")
  png_renderer(filename = full_filename, width = pixel_size, height = pixel_size, units = "px")
  plot(g)
  dev.off()

  elapsed_time <- as.numeric(proc.time() - start_time)[3]
  tibble(n_points = n_points, renderer = renderer_name, elapsed_time = elapsed_time)
}

measure_time_to_draw <- function(log2_ns, pixel_size, int_coord) {
  df_time <- purrr::map(log2_ns, function(n) {
    set.seed(n)
    fern <- make_fern(n)
    png_result <- draw_fern(
      fern = fern, png_renderer = png,
      renderer_name = "png", file_basename = "bernsley_fern_png",
      pixel_size = pixel_size, int_coord = int_coord
    )

    cairo_result <- draw_fern(
      fern = fern, png_renderer = Cairo::CairoPNG,
      renderer_name = "Cairo", file_basename = "bernsley_fern_cairo",
      pixel_size = pixel_size, int_coord = int_coord
    )

    dplyr::bind_rows(cairo_result, png_result)
  }) %>%
    dplyr::bind_rows()

  df_time_wide <- df_time %>%
    tidyr::pivot_wider(names_from = renderer, values_from = elapsed_time) %>%
    dplyr::mutate(ratio = png / Cairo)

  df_time_long <- df_time %>%
    dplyr::mutate(renderer = factor(renderer)) %>%
    dplyr::mutate(renderer = forcats::fct_relevel(renderer, c("png", "Cairo")))

  g <- ggplot(df_time_long)
  g <- g + geom_line(aes(x = n_points, y = elapsed_time, color = renderer, linetype = renderer))
  g <- g + scale_color_manual(values = c("navy", "orange"))
  g <- g + scale_linetype_manual(values = c("dashed", "solid"))
  g <- g + xlab("# of points")
  g <- g + ylab("time to write a PNG file [second]")
  g <- g + theme_bw()
  g <- g + theme(
    text = element_text(family = g_font_name, size = 8),
    legend.text = element_text(size = 8),
    legend.title = element_blank(),
    axis.text = element_text(family = g_font_name, size = 6),
    axis.title = element_text(family = g_font_name, size = 6),
    strip.text = element_text(family = g_font_name, size = 8),
    plot.title = element_text(family = g_font_name, size = 8)
  )
  ggsave(filename = "fern_time.png", plot = g, width = 960, height = 540, units = "px")
}

measure_time_to_draw(log2_ns = 14:18, pixel_size = 256, int_coord = FALSE)
