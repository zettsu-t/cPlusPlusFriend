## Drawing the Barnsley fern
library(tidyverse)
library(abind)
library(assertthat)
library(raster)

## Based on
## https://jp.mathworks.com/matlabcentral/communitycontests/contests/4/entries/586
g_next_edges <- c(rep.int(x = 1:4, times = c(1, 85, 7, 7)))
invisible(assertthat::assert_that(NROW(g_next_edges) == 100))

g_transform_set <- array(
  data = c(
    0.0, 0.85, -0.15, 0.20,
    0.0, -0.04, 0.26, 0.23,
    0.0, 0.04, 0.28, -0.26,
    0.16, 0.85, 0.24, 0.22
  ),
  dim = c(4, 2, 2)
)

g_translate_set <- array(
  data = c(
    0.0, 0.0, 0.0, 0.0,
    0.0, 1.6, 1.6, 0.44
  ),
  dim = c(4, 1, 2)
)

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
  purrr::map(seq_len(n_points), function(i) {
    next_points[i, , ] <<- point_transform_set$transform[i, , ] %*% points[i, , , drop = FALSE]
    NULL
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

draw_fern <- function(fern) {
  xs <- fern$points[,,1]
  ys <- fern$points[,,2]
  df <- tibble::tibble(x = xs, y = ys)

  g <- ggplot(df)
  g <- g + geom_point(aes(x = x, y = y), color = "mediumblue", size = 0.2, alpha = 0.6, shape = 8)
  g <- g + theme_light()
  g <- g + theme(
    aspect.ratio = 1.0, panel.background = element_rect(fill = "wheat2"),
    panel.border = element_blank(), axis.ticks = element_blank(),
    panel.grid.major = element_blank(), panel.grid.minor = element_blank(),
    axis.text = element_blank(), axis.title = element_blank()
  )
  ggsave(filename = "bernsley_fern.png", plot = g, width = 512, height = 512, units = "px")
  g
}

fern <- make_fern(13)
img <- draw_fern(fern)
