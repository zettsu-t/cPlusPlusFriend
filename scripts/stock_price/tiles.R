library(tidyverse)

data_to_long_dataframe <- function(vs, width) {
  mat <- matrix(vs, ncol = width, byrow = TRUE)
  colnames(mat) <- purrr::map_chr(seq_len(NCOL(mat)), ~ paste0("V", .x))
  df <- as_tibble(mat)
  x_colnames <- colnames(df)

  df %>%
    dplyr::mutate(y = row_number()) %>%
    tidyr::pivot_longer(cols = x_colnames, names_to = "x") %>%
    dplyr::mutate(x = as.numeric(stringr::str_replace(x, "\\D", ""))) %>%
    dplyr::mutate(x = 1 + NCOL(.) - x) %>%
    dplyr::sample_n(NROW(.))
}

make_tile_dataframe <- function() {
  width <- 32
  height <- 18
  value_min <- -2
  value_max <- 4
  value_set <- sort(runif(n = width * height - 2, min = value_min, max = value_max))
  na_indexes <- sample(seq_len(NROW(value_set)), ceiling(width * 1.3), replace = FALSE)
  value_set[na_indexes] <- NA
  data_to_long_dataframe(vs = c(value_min, value_set, value_max), width = width)
}

make_a_font_dataframe <- function() {
  indicators <- c(1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0)
  cumsum_values <- cumsum(indicators)
  min_value <- 1
  max_value <- max(cumsum_values)
  mid_value <- mean(c(min_value, max_value))
  values <- ((cumsum_values - mid_value) / (max_value - min_value)) * indicators
  values <- ifelse(abs(values) < 1e-7, rep(NA, NROW(values)), values)
  data_to_long_dataframe(vs = values, width = 4)
}

normalize_abs <- function(raw_values, color_high, color_low, color_middle) {
  minmax_values <- range(raw_values, na.rm = TRUE)
  min_value <- minmax_values[1]
  max_value <- minmax_values[2]

  to_colors <- function(normalized_values, color_start, color_end) {
    purrr::map_chr(normalized_values, function(x) {
      if (is.na(x)) {
        NA
      } else {
        rgb(colorRamp(c(color_start, color_end))(x) / 255)
      }
    })
  }

  if (min_value == max_value) {
    to_colors(rep(0, NROW(raw_values)), color_middle, color_high)
  } else if (min_value > 0) {
    to_colors((raw_values - min_value) / (max_value - min_value), color_middle, color_high)
  } else if (max_value < 0) {
    to_colors((max_value - raw_values) / (max_value - min_value), color_middle, color_low)
  } else {
    colors_positive <- to_colors(pmax(raw_values, 0) / max_value, color_middle, color_high)
    colors_negative <- to_colors(pmin(raw_values, 0) / min_value, color_middle, color_low)
    purrr::map_chr(seq_len(NROW(raw_values)), function(i) {
      x <- raw_values[[i]]
      if (is.na(x)) {
        NA
      } else if (x >= 0) {
        colors_positive[[i]]
      } else {
        colors_negative[[i]]
      }
    })
  }
}

draw_color_tiles <- function(df, color_high, color_low, color_middle, aspect_ratio) {
  df_drawn <- df %>%
    dplyr::arrange(value) %>%
    dplyr::mutate(col = normalize_abs(
      value, color_high = color_high, color_low = color_low, color_middle = color_middle)) %>%
    dplyr::mutate(index = row_number()) %>%
    na.omit()

  minmax_values <- range(df_drawn$value, na.rm = TRUE)
  minmax_indexes <- range(df_drawn$index, na.rm = TRUE)

  g <- ggplot(df_drawn)
  g <- g + geom_tile(aes(x=x, y=y, fill=index), color = "black")
  g <- g + scale_fill_gradientn("Value", colours = df_drawn$col, na.value = "transparent",
                                labels = minmax_values, breaks = minmax_indexes, limits = minmax_indexes)
  g <- g + theme_bw()
  if (!is.na(aspect_ratio)) {
    g <- g + theme(aspect.ratio = aspect_ratio)
  }
  plot(g)
}

color_high <- "royalblue1"
color_low <- "orange"
color_middle <- "gray80"

df_tiles <- make_tile_dataframe()
g <- ggplot(df_tiles)
g <- g + geom_tile(aes(x=x, y=y, fill=value), color = "black")
g <- g + scale_fill_gradient2(low = color_low, mid = color_middle,
                              high = color_high, na.value = "transparent")
g <- g + theme_bw()
plot(g)

draw_color_tiles(df = df_tiles, color_high = color_high, color_low = color_low,
                 color_middle = color_middle, aspect_ratio = NA)

df_a_font <- make_a_font_dataframe()
draw_color_tiles(df = df_a_font, color_high = color_high, color_low = color_low,
                 color_middle = color_middle, aspect_ratio = 1.0)
