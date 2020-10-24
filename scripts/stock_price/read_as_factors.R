library(tidyverse)
library(assertthat)
library(data.table)
library(makedummies)
library(microbenchmark)
library(pryr)

## Delete manually
filename <- "_chars.csv.gz"

items <- c("foo", "bar", "hoge", "hya")
n_items <- 10000000

generate_items <- function(n) {
    sample(items, n, replace=TRUE)
}

strings_to_factors <- function(df) {
    df %>%
        dplyr::mutate(across(everything(), as.factor))
}

df <- tibble(p=generate_items(n_items), q=generate_items(n_items), r=generate_items(n_items))
result <- microbenchmark(strings_to_factors(df), unit='s', times=2)
print(result)
print(pryr::mem_used())

readr::write_csv(df, filename)
df <- NULL

read_by_fread <- function(filename) {
    as_tibble(data.table::fread(filename, stringsAsFactors=TRUE))
}

read_by_read_csv <- function(filename) {
    readr::read_csv(filename) %>%
        dplyr::mutate_all(as.factor)
}

measure <- function(reader, filename) {
    df <- suppressMessages(reader(filename))
    assertthat::assert_that(all(purrr::map_lgl(1:NCOL(df), ~ class(df[[1,.x]]) == "factor")))
}

gc()
result_fread <- microbenchmark(measure(read_by_fread, filename), unit='s', times=2)
print(result_fread)

gc()
result_read_csv <- microbenchmark(measure(read_by_read_csv, filename), unit='s', times=2)
print(result_read_csv)

warning(paste(filename, "left"))
file.info(filename)
