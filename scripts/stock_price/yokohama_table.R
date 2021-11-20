## Data source used in this script:
## https://www.city.yokohama.lg.jp/city-info/yokohamashi/tokei-chosa/portal/opendata/
library(tidyverse)
library(extrafont)
library(DT)
library(gt)

get_color_index <- function(wards, indexes) {
  purrr::map2_dbl(wards, indexes, function(ward, index) {
    if (ward == "横浜市") {
      1
    } else {
      2 + (index %% 2)
    }
  })
}

to_color_table <- function(df) {
  df_gt <- df %>%
    dplyr::mutate(index = row_number()) %>%
    dplyr::mutate(color = get_color_index(市区名, index)) %>%
    gt::gt() %>%
    gt::cols_hide(c("index", "color")) %>%
    gt::tab_style(
      style = cell_fill(color = "gray70"),
      locations = cells_body(rows = (color == 1))
    ) %>%
    gt::tab_style(
      style = cell_fill(color = "azure"),
      locations = cells_body(rows = (color == 3))
    )

  purrr::reduce(.x=4:12, .init=df_gt, function(acc, col) {
    offset <- 1
    ward_max <- offset + which.max(as.vector(unlist(df_population_data[-offset, col])))
    ward_min <- offset + which.min(as.vector(unlist(df_population_data[-offset, col])))

    acc %>% gt::tab_style(
      style = cell_fill(color = 'khaki1'),
      locations = cells_body(
        columns = c(col),
        rows = c(ward_max)
      )) %>% gt::tab_style(
      style = cell_fill(color = 'khaki3'),
      locations = cells_body(
        columns = c(col),
        rows = c(ward_min)
      ))
  }) %>%
    gt::opt_table_font(font = "Migu 1M")
}

df_population_data <- readr::read_csv("incoming_yokohama/e1yokohama2009.csv")
to_color_table(df_population_data)
DT::datatable(df_population_data)
