library(tidyverse)
library(viridis)

source("juliaset.R")

converge <- function(init, offset, max_iter, limit) {
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
    dplyr::mutate(mod_2_diff = abs(mod_2 - lag(mod_2, n=1))) %>%
    dplyr::mutate(under = mod < limit)

  n_row <- max(which(df$under))
  if (is.infinite(n_row)) {
    n_row <- 0
  }
  df_top <- df %>%
    slice(seq_len(min(NROW(df), (n_row + 1))))

  print(df_top$i[max(which(df_top$under))])
  df_top
}

large <- complex(real = 23.0, imaginary = 0.0)
invisible(converge(init = large, offset = large, max_iter = 10, limit = 2.0))
one_zero <- complex(real = 1.0 + 1e-7, imaginary = 0.0)
zero_one <- complex(real = 0.0, imaginary = 1.0 + 1e-7)
zero <- complex(real = 0, imaginary = 0)
invisible(converge(init = one_zero, offset = zero, max_iter = 1000, limit = 2.0))
invisible(converge(init = zero_one, offset = zero, max_iter = 1000, limit = 2.0))

offset_a <- complex(real = 0.5, imaginary = 0.375)
offset_b <- complex(real = 0.375, imaginary = 0.5)
offset_c <- complex(real = 0.375, imaginary = 0.375)
offset_d <- complex(real = 0.5, imaginary = 0.5)
invisible(converge(init = zero, offset = offset_a, max_iter = 1000, limit = 2.0))
invisible(converge(init = zero, offset = offset_b, max_iter = 1000, limit = 2.0))
invisible(converge(init = offset_a, offset = offset_b, max_iter = 1000, limit = 2.0))
invisible(converge(init = offset_b, offset = offset_a, max_iter = 1000, limit = 2.0))
invisible(converge(init = offset_c, offset = offset_c, max_iter = 1000, limit = 2.0))

invisible(converge(init = offset_c, offset = offset_c, max_iter = 1000, limit = 2.0))
invisible(converge(init = offset_b, offset = offset_c, max_iter = 1000, limit = 2.0))

invisible(converge(init = offset_c, offset = offset_a, max_iter = 11198, limit = 2.0))
invisible(converge(init = offset_a, offset = offset_a, max_iter = 1118, limit = 2.0))

# scan_points(x_offset = 0.25, y_offset = 0.75, max_iter = 3, n_pixels = 4)

df <- readr::read_csv("cpp.csv", col_names = FALSE) %>%
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

machine_eps <- 1.19e-7
sqrt_eps <- sqrt(machine_eps)
sprintf("%.20f", machine_eps)

converge_point(23.0, 0.0, complex(re=0.0, im=0.0), 100, sqrt_eps)
converge_point(1.0 + machine_eps, 0.0, complex(re=0.0, im=0.0), 100, sqrt_eps)

converge_point(0.0, 0.0, complex(re=0.5, im=0.375), 100, sqrt_eps)
converge_point(0.0, 0.0, complex(re=0.375, im=0.5), 100, sqrt_eps)
converge_point(0.5, 0.375, complex(re=0.375, im=0.5), 100, sqrt_eps)
converge_point(0.375, 0.5, complex(re=0.5, im=0.375), 100, sqrt_eps)
converge_point(0.375, 0.375, complex(re=0.375, im=0.375), 100, 0.1)
converge_point(0.375, 0.375, complex(re=0.375, im=0.375), 11, 0.1)

to_table <- function(base_n) {
  purrr::reduce(.x = seq(99), .init = list(df = NULL, n = base_n), .f = function(acc, i) {
    n <- acc$n
    mod_n <- sprintf("%.20f", Mod(n))

    next_n <- n * n
    diff_n <- Mod(next_n - n)
    mod_diff <- sprintf("%.20f", diff_n)
    mod_diff_sqr <- sprintf("%.20f", diff_n ** 2)

    df <- tibble::tibble(i = i, mod = mod_n, diff = mod_diff, diff_sqr = mod_diff_sqr)
    next_df <- if (is.null(acc$df)) {
      df
    } else {
      dplyr::bind_rows(acc$df, df)
    }
    list(df = next_df, n = next_n)
  })$df
}

df_eps <- to_table(base_n = complex(re = 0.9, im = 0.0))
