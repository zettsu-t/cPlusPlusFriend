library(tidyverse)

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
