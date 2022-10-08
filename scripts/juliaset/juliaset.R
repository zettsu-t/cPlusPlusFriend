library(tidyverse)
library(raster)
library(viridis)

#' Transform a point under the rule of Julia sets
#'
#' @param from An original point
#' @param offset An offset to be added
#' @return The transformed point under the rule of Julia sets
transform_point <- function(from, offset) {
  from * from + offset
}

#' Count how many times a point is transformed
#'
#' @param point_x The x coordinate of a point
#' @param point_y The y coordinate of a point
#' @param offset An offset to be added
#' @param max_iter The maximum number of iterations
#' @param eps Tolerance to check if transformations are converged
#' @return How many times a point is transformed
converge_point <- function(point_x, point_y, offset, max_iter, eps) {
  z <- complex(real = point_x, imaginary = point_y)
  count <- 0
  previous_modulus <- Inf

  while (count < max_iter) {
    next_z <- transform_point(z, offset)
    z_modulus <- Mod(next_z)
    if (z_modulus > 2.0) {
      break
    }

    if (Mod(next_z - z) < eps) {
      break
    }

    count <- count + 1
    z <- next_z
  }
  count
}

#' Count how many times each point in a screen is transformed
#'
#' @param xs X coordinates of points in a screen
#' @param ys Y coordinates of points in a screen
#' @param offset An offset to be added
#' @param max_iter The maximum number of iterations
#' @param eps Tolerance to check if transformations are converged
#' @return How many times each point in a screen is transformed
converge_point_set <- function(xs, ys, offset, max_iter, eps) {
  mat_counts <- matrix(0, nrow = NROW(ys), ncol = NROW(xs))
  purrr::walk(seq_len(NROW(ys)), function(y_index) {
    purrr::walk(seq_len(NROW(xs)), function(x_index) {
      point_x <- xs[x_index]
      point_y <- ys[y_index]
      mat_counts[y_index, x_index] <<- converge_point(
        point_x = point_x, point_y = point_y, offset = offset, max_iter = max_iter, eps = eps
      )
    })
  })
  mat_counts
}

#' Calculate pixel coordinates on an axis in a screen
#'
#' @param half_length Maximum x and y coordinates relative to (0,0)
#' @param n_pixels Numbers of pixels in X and Y axes
#' @return Pixel coordinates on an axis in a screen
map_coordinates <- function(half_length, n_pixels) {
  seq(from = -half_length, to = half_length, length.out = n_pixels)
}

#' Scan how many times each point in a screen is transformed
#'
#' @param x_offset An x offset to be added
#' @param y_offset A y offset to be added
#' @param max_iter The maximum number of iterations
#' @param width The number of pixels in the X axes
#' @param height The number of pixels in the Y axes
#' @return How many times each point in a screen is transformed
scan_points <- function(x_offset, y_offset, max_iter, width, height = NA) {
  n_pixels_y <- if (is.na(height)) {
    width
  } else {
    height
  }

  half_length <- sqrt(2.0) + 0.1
  xs <- map_coordinates(half_length = half_length, n_pixels = width)
  ys <- map_coordinates(half_length = half_length, n_pixels = n_pixels_y)
  offset <- complex(real = x_offset, imaginary = y_offset)
  eps <- sqrt(1.19e-7)
  converge_point_set(xs = xs, ys = ys, offset = offset, max_iter = max_iter, eps = eps)
}

#' Draw a PNG image from an input screen
#'
#' @param count_set Counts of a Julia set in a screen
#' @param color_func A color function to fill pixels
#' @param png_filename An output PNG filename
#' @return A PNG image from an input screen
draw_image <- function(count_set, color_func, png_filename = NA) {
  shape <- c(NROW(count_set), NCOL(count_set))
  mat_color <- array(0.0, c(shape, 3))

  df_color <- tibble::tibble(color = color_func(n = diff(range(count_set)) + 1)) %>%
    dplyr::mutate(color = stringr::str_replace_all(color, "#?(..)", "\\1_")) %>%
    tidyr::separate(col = "color", into = c("r", "g", "b", "a", "other"), sep = "_") %>%
    dplyr::mutate_all(function(x) {
      strtoi(x, 16)
    })

  mat_color[, , 1] <- array(df_color$r[count_set + 1], shape)
  mat_color[, , 2] <- array(df_color$g[count_set + 1], shape)
  mat_color[, , 3] <- array(df_color$b[count_set + 1], shape)
  mat_color <- mat_color / 255.0
  img <- as.raster(mat_color)

  if (!is.na(png_filename)) {
    png(png_filename, height = shape[1], width = shape[2])
    par(mar = c(0, 0, 0, 0))
    plot(img)
    dev.off()
  }

  img
}

draw_samples <- function() {
  count_set <- scan_points(x_offset = 0.382, y_offset = 0.382, max_iter = 75, width = 256)
  plot(draw_image(
    count_set = count_set, color_func = viridis::magma,
    png_filename = "juliaset_r_square.png"
  ))

  count_set <- scan_points(x_offset = 0.382, y_offset = 0.382, max_iter = 75, width = 256, height = 128)
  plot(draw_image(
    count_set = count_set, color_func = viridis::magma,
    png_filename = "juliaset_r_landscape.png"
  ))

  count_set <- scan_points(x_offset = 0.382, y_offset = 0.382, max_iter = 75, width = 128, height = 256)
  plot(draw_image(
    count_set = count_set, color_func = viridis::magma,
    png_filename = "juliaset_r_portrait.png"
  ))

  count_set <- scan_points(x_offset = 0.25, y_offset = -0.75, max_iter = 1000, width = 4, height = 5)
  count_set <- scan_points(x_offset = -0.25, y_offset = 0.75, max_iter = 1000, width = 3, height = 4)
  count_set <- scan_points(x_offset = -0.375, y_offset = -0.75, max_iter = 1000, width = 5, height = 3)
}

count_set <- draw_samples()

pixel_filename <- "rust/juliaset/rust_juliaset.csv"
if (file.exists(pixel_filename)) {
  count_rust <- as.matrix(read.table(pixel_filename, header = FALSE, sep = ","))
  plot(draw_image(
    count_set = count_rust, color_func = viridis::mako,
    png_filename = "juliaset_rust.png"
  ))
}
