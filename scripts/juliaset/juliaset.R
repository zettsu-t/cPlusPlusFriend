library(tidyverse)
library(raster)
library(viridis)

transform_point <- function(z, offset) {
  z * z + offset
}

converge_point <- function(point_x, point_y, offset, max_iter, eps) {
  z <- complex(real = point_x, imaginary = point_y)
  count <- 0
  previous_modulus <- Inf

  while (count < max_iter) {
    z <- transform_point(z, offset)
    z_modulus <- Mod(z)
    if (z_modulus > 2.0) {
      break
    }
    if (abs(previous_modulus - z_modulus) < eps) {
      break
    }

    count <- count + 1
    previous_modulus <- z_modulus
  }
  count
}

converge_point_set <- function(offset, max_iter, eps, xs, ys) {
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

scan_points <- function(x_offset, y_offset, max_iter, n_pixels) {
  abs_length <- sqrt(2) + 0.1
  xs <- seq(from = -abs_length, to = abs_length, length.out = n_pixels)
  ys <- seq(from = -abs_length, to = abs_length, length.out = n_pixels)
  offset <- complex(real = x_offset, imaginary = y_offset)
  eps <- 1e-5
  mat_count <- converge_point_set(offset = offset, max_iter = max_iter, eps = eps, xs = xs, ys = ys)
  list(xs = xs, ys = ys, mat_count = mat_count)
}

create_image <- function(mat, color_func, png_filename = NA) {
  mat_color <- array(0.0, c(NCOL(mat), NROW(mat), 3))

  df_color <- tibble::tibble(color = color_func(n = diff(range(mat)) + 1)) %>%
    dplyr::mutate(color = stringr::str_replace_all(color, "#?(..)", "\\1_")) %>%
    tidyr::separate(col = "color", into = c("r", "g", "b", "a", "other"), sep = "_") %>%
    dplyr::mutate_all(function(x) {
      strtoi(x, 16)
    })

  mat_color[, , 1] <- array(df_color$r[mat + 1], c(NCOL(mat), NROW(mat)))
  mat_color[, , 2] <- array(df_color$g[mat + 1], c(NCOL(mat), NROW(mat)))
  mat_color[, , 3] <- array(df_color$b[mat + 1], c(NCOL(mat), NROW(mat)))
  mat_color <- mat_color / 255.0
  img <- as.raster(mat_color)

  if (!is.na(png_filename)) {
    png(png_filename, height = dim(mat)[1], width = dim(mat)[2])
    plot(img)
    dev.off()
  }

  img
}

result <- scan_points(x_offset = 0.382, y_offset = 0.382, max_iter = 75, n_pixels = 1000)
plot(create_image(mat = result$mat_count, color_func = viridis::magma, png_filename = "juliaset_r.png"))

mat_rust <- as.matrix(read.table("rust/juliaset/rust_juliaset.csv",  header = FALSE, sep = ","))
plot(create_image(mat_rust, color_func = viridis::mako, png_filename = "juliaset_rust.png"))
