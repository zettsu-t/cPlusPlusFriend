library(tidyverse)
library(viridis)

source("juliaset.R")

## test_transform_point()
p <- complex(re = 2.0, im = 4.0)
q <- complex(re = 3.0, im = 5.0)
p * p + q

## test_converge_point()
machine_eps <- 1.19e-7
sqrt_eps <- sqrt(machine_eps)
sprintf("%.20f", machine_eps)

## Min count
converge_point(23.0, 0.0, complex(re = 0.0, im = 0.0), 100, sqrt_eps)
## Eps
converge_point(0.9, 0.0, complex(re = 0.0, im = 0.0), 100, sqrt(1e-3))
## Converged
converge_point(0.375, 0.375, complex(re = 0.375, im = 0.375), 100, 0.1)
## Max count
converge_point(0.375, 0.375, complex(re = 0.375, im = 0.375), 11, 0.1)
## The machine epsion
converge_point(1.0 + machine_eps, 0.0, complex(re = 0.0, im = 0.0), 100, sqrt_eps)
## Some converged cases
converge_point(0.0, 0.0, complex(re = 0.5, im = 0.375), 100, sqrt_eps)
converge_point(0.0, 0.0, complex(re = 0.375, im = 0.5), 100, sqrt_eps)
converge_point(0.5, 0.375, complex(re = 0.375, im = 0.5), 100, sqrt_eps)
converge_point(0.375, 0.5, complex(re = 0.5, im = 0.375), 100, sqrt_eps)

## test_converge_point_set_row()
## test_converge_point_set_column()
converge_point(0.375, 0.375, complex(re = 0.375, im = 0.375), 100, 0.1)
converge_point(0.5, 0.375, complex(re = 0.375, im = 0.375), 100, 0.1)
converge_point(0.375, 0.5, complex(re = 0.375, im = 0.375), 100, 0.1)

converge_point(0.375, 0.375, complex(re = 0.5, im = 0.375), 100, 0.1)
converge_point(0.5, 0.375, complex(re = 0.5, im = 0.375), 100, 0.1)

to_table <- function(base_n, max_iter) {
  purrr::reduce(.x = seq(max_iter), .init = list(df = NULL, n = base_n), .f = function(acc, i) {
    n <- acc$n
    mod_n <- sprintf("%.20f", Mod(n))

    next_n <- n * n
    diff_n <- Mod(next_n - n)
    mod_diff <- sprintf("%.20f", diff_n)
    mod_diff_sqr <- sprintf("%.20f", diff_n**2)

    df <- tibble::tibble(i = i, mod = mod_n, diff = mod_diff, diff_sqr = mod_diff_sqr)
    next_df <- if (is.null(acc$df)) {
      df
    } else {
      dplyr::bind_rows(acc$df, df)
    }
    list(df = next_df, n = next_n)
  })$df
}

df_eps <- to_table(base_n = complex(re = 0.9, im = 0.0), max_iter = 11)

iterate_to_converge <- function(init, offset, max_iter, limit, eps) {
  f <- function(from) {
    from * from + offset
  }

  df <- purrr::reduce(.x = 0:max_iter, .init = NULL, .f = function(acc, i) {
    if (is.null(acc)) {
      tibble::tibble(i = i, x = init)
    } else {
      x <- tail(acc$x, 1)
      dplyr::bind_rows(acc, tibble::tibble(i = i, x = f(x)))
    }
  }) %>%
    dplyr::mutate(mod = Mod(x)) %>%
    dplyr::mutate(mod_2 = mod * mod) %>%
    dplyr::mutate(mod_2_diff = abs(mod_2 - lag(mod_2, n = 1))) %>%
    dplyr::mutate(mod_2_diff = ifelse(is.na(mod_2_diff), mod_2, mod_2_diff)) %>%
    dplyr::mutate(under_limit = mod < limit) %>%
    dplyr::mutate(under_eps = mod_2_diff < eps)

  n_row <- max(which(df$under_limit))
  if (is.infinite(n_row)) {
    n_row <- 0
  }
  df %>%
    slice(seq_len(min(NROW(df), (n_row + 1))))
}

limit <- 2.0
max_iter <- 100
iterate_to_converge(
  init = complex(re = 23.0, im = 0.0), offset = complex(re = 0.0, im = 0.0),
  max_iter = max_iter, limit = limit, eps = machine_eps
)
iterate_to_converge(
  init = complex(re = 0.9, im = 0.0), offset = complex(re = 0.0, im = 0.0),
  max_iter = max_iter, limit = limit, eps = 1e-3
)
iterate_to_converge(
  init = complex(re = 0.375, im = 0.375), offset = complex(re = 0.375, im = 0.375),
  max_iter = max_iter, limit = limit, eps = 0.01
)
iterate_to_converge(
  init = complex(re = 1.0 + machine_eps, im = 0.0), offset = complex(re = 0.0, im = 0.0),
  max_iter = max_iter, limit = sqrt_eps, eps = 0.01
)
iterate_to_converge(
  init = complex(re = 0.0, im = 0.0), offset = complex(re = 0.5, im = 0.375),
  max_iter = max_iter, limit = limit, eps = machine_eps
)
iterate_to_converge(
  init = complex(re = 0.0, im = 0.0), offset = complex(re = 0.375, im = 0.5),
  max_iter = max_iter, limit = limit, eps = machine_eps
)
iterate_to_converge(
  init = complex(re = 0.5, im = 0.375), offset = complex(re = 0.375, im = 0.5),
  max_iter = max_iter, limit = limit, eps = machine_eps
)
iterate_to_converge(
  init = complex(re = 0.375, im = 0.5), offset = complex(re = 0.5, im = 0.375),
  max_iter = max_iter, limit = limit, eps = machine_eps
)

iterate_to_converge(
  init = complex(re = 0.375, im = 0.375), offset = complex(re = 0.375, im = 0.375),
  max_iter = max_iter, limit = limit, eps = machine_eps
)
iterate_to_converge(
  init = complex(re = 0.5, im = 0.375), offset = complex(re = 0.375, im = 0.375),
  max_iter = max_iter, limit = limit, eps = machine_eps
)
iterate_to_converge(
  init = complex(re = 0.375, im = 0.375), offset = complex(re = 0.375, im = 0.375),
  max_iter = max_iter, limit = limit, eps = machine_eps
)
iterate_to_converge(
  init = complex(re = 0.375, im = 0.5), offset = complex(re = 0.375, im = 0.375),
  max_iter = max_iter, limit = limit, eps = machine_eps
)
iterate_to_converge(
  init = complex(re = 0.375, im = 0.5), offset = complex(re = 0.5, im = 0.375),
  max_iter = max_iter, limit = limit, eps = machine_eps
)
iterate_to_converge(
  init = complex(re = 0.5, im = 0.375), offset = complex(re = 0.5, im = 0.375),
  max_iter = max_iter, limit = limit, eps = machine_eps
)

## Draw C++ impl outputs
## cd cpp
## mkdir -p build
## cmake ..
## cmake build
## ./juliaset -c cpp.csv
df <- readr::read_csv("cpp/build/cpp.csv", col_names = FALSE) %>%
  dplyr::mutate(y = row_number()) %>%
  tidyr::pivot_longer(starts_with("X")) %>%
  tidyr::separate(col = name, into = c("pre", "x"), sep = "\\D+") %>%
  dplyr::mutate(x = as.numeric(x), y = as.numeric(y))

df_drawn <- df %>%
  dplyr::arrange(value) %>%
  dplyr::mutate(value = factor(value))

colors <- viridis::magma(max(df$value) + 1)

g <- ggplot(df_drawn)
g <- g + geom_tile(aes(x = x, y = y, fill = value))
g <- g + scale_fill_manual(values = colors)
g <- g + theme(legend.position = "none", aspect.ratio = 1.0)
plot(g)

n_colors <- 65
extract_rgb <- function(n_colors, offset) {
  index <- 2 + offset * 2
  viridis::cividis(n_colors) %>%
    stringr::str_sub(index, index + 1) %>%
    base::strtoi(16)
}

red_set <- extract_rgb(n_colors = n_colors, offset = 0)
diff(red_set)
green_set <- extract_rgb(n_colors = n_colors, offset = 1)
diff(green_set)
blue_set <- extract_rgb(n_colors = n_colors, offset = 2)
diff(blue_set)

half_length <- sqrt(2) + 0.1
seq(from = -half_length, to = half_length, length.out = 5)

scan_points(0.25, -0.75, 100, 4, 5)
scan_points_xy(-0.625, 0.75, 1000, c(-0.25, -0.125, -0.0625), c(0.375, 0.5, 0.625, 0.75))
