library(tidyverse)
library(purrrlyr)
library(htmlTable)
library(openxlsx)

filename <- "fizzbuzz.xlsx"

df_original <- tibble(num = 1:30) %>%
  dplyr::mutate(fizz = ifelse((num %% 3) == 0, "Fizz", ""),
                buzz = ifelse((num %% 5) == 0, "Buzz", "")) %>%
  tidyr::unite(str, fizz, buzz, sep = "") %>%
  dplyr::mutate(str = ifelse(stringr::str_length(str) == 0, as.character(num), str))

## Make a spreadsheet
new_wb <- openxlsx::createWorkbook()
openxlsx::addWorksheet(wb = new_wb, sheetName = "fizzbuzz")
openxlsx::writeData(wb = new_wb, sheet = 1, x = df_original)

fizz_style <- openxlsx::createStyle(fgFill = "royalblue1")
buzz_style <- openxlsx::createStyle(fgFill = "orange1")
fizz_buzz_style <- openxlsx::createStyle(fgFill = "gray70")

invisible(purrrlyr::by_row(df_original, function(x) {
  style <- NULL
  str <- x$str
  if (str == "Fizz") {
    style <- fizz_style
  } else if (str == "Buzz") {
    style <- buzz_style
  } else if (str == "FizzBuzz") {
    style <- fizz_buzz_style
  }

  if (!is.null(style)) {
    openxlsx::addStyle(wb = new_wb, sheet = 1, style = style, rows = 1 + x$num, cols = c(1,2))
  }
  0
}))

saveWorkbook(wb = new_wb, file = filename, overwrite = TRUE)

## Read a spreadsheet
wb_restored <- openxlsx::loadWorkbook(file = filename)
df_restored <- openxlsx::readWorkbook(filename, sheet = 1)

## Get cell colors
## Based on
## https://stackoverflow.com/questions/62519400/filter-data-highlighted-in-excel-by-cell-fill-color-using-openxlsx
df_cells <- (purrr::map(wb_restored$styleObjects, function(x) {
  if (!is.null(x$style$fill$fillFg)) {
    fg_color <- x$style$fill$fillFg
    rows <- x$rows - 1
    cols <- x$cols
    tibble(fg_color = fg_color, row = rows, col = cols)
  }
}) %>%
  dplyr::bind_rows() %>%
  dplyr::arrange(col, row) %>%
  purrrlyr::by_row(function(x) {
    str <- as.character(df_restored[x$row, x$col])
    tibble(str = str, row = x$row, col = x$col, fg_color = x$fg_color)
  }, .collate = c("list")))$.out %>%
  dplyr::bind_rows()

df_color_cells <- df_cells %>%
  dplyr::filter(stringr::str_detect(str, "\\D")) %>%
  dplyr::select(str, fg_color) %>%
  dplyr::distinct()

## Based on
## https://stackoverflow.com/questions/31323885/how-to-color-specific-cells-in-a-data-frame-table-in-r
css_cell <- purrr::reduce(seq_len(NROW(df_cells)),
                          .init = matrix('', NROW(df_restored), NCOL(df_restored)),
                          function(acc, row_num) {
  x <- df_cells[row_num,]
  bg_color <- stringr::str_extract(x$fg_color, ".{6}$")
  style <- paste0("background-color: ", bg_color, ";")
  acc[x$row, x$col] <- style
  acc
})

htmlTable(df_restored, css.cell = css_cell)
