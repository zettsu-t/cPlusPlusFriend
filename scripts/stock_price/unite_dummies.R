library(tidyverse)
library(assertthat)
library(caret)
library(makedummies)
library(purrrlyr)

make_dummies_united_dummies <- function(df, cols, method, full_rank, to_factor) {
  df_united <- df %>%
    dplyr::select(all_of(cols)) %>%
    tidyr::unite("value", everything(), sep = "::")

  ## makedummies::makedummies does not convert characters to dummies
  if (to_factor) {
    df_united <- df_united %>%
      dplyr::mutate(value = factor(value))
  }

  n_patterns <- NROW(dplyr::distinct(df_united))

  df_dummies <- if (method == "caret") {
    predict(caret::dummyVars(~ ., data = df_united, sep = "..",
                             levelsOnly = FALSE, fullRank = full_rank), df_united)
  } else {
    makedummies::makedummies(df_united, basal_level = !full_rank)
  }

  assert_that(assertthat::are_equal(NROW(df_united), NROW(df_dummies)))
  assert_that(assertthat::are_equal(NCOL(df_united), 1))
  if (to_factor) {
    assert_that(assertthat::are_equal(NCOL(df_dummies), n_patterns - full_rank))
  }
  paste0(colnames(df_dummies), collapse = ",")
}

make_all_dummies <- function(df, to_factor) {
  df_froms <- tibble(from = colnames(df))
  df_tos <- df_froms %>%
    dplyr::rename(to = from)
  ## Create a Cartesian product of froms and tos
  df_from_tos <- dplyr::full_join(df_froms, df_tos, by = character())

  make_dummies <- function(arg_df, arg_method, arg_full_rank, arg_to_factor) {
    purrrlyr::by_row(df_from_tos, function(input) {
      cols <- unique(as.vector(unlist(input[1,])))
      make_dummies_united_dummies(df = arg_df, cols = cols,
        method = arg_method, full_rank = arg_full_rank, to_factor = arg_to_factor)
    }, .collate = "cols", .to = "colnames") %>%
      dplyr::bind_rows() %>%
      dplyr::mutate(method = arg_method, full_rank = arg_full_rank, to_factor = arg_to_factor)
  }

  df_result <- purrr::map(c("makedummies", "caret"), function(method) {
    purrr::map(c(FALSE, TRUE), function(full_rank) {
      make_dummies(arg_df = df, arg_method = method, arg_full_rank = full_rank, arg_to_factor = to_factor)
    }) %>%
      dplyr::bind_rows()
  }) %>%
    dplyr::bind_rows()
}

make_example <- function(to_factor) {
  df_all <- tibble(x1 = c("a", "a", "a", "b", "b", "b", "b"),
                   x2 = c("f", "f", "f", "g", "g", "h", "h"),
                   x3 = c("p", "q", "p", "q", "p", "q", "p"),
                   x4 = c("s1", "s2", "s3", "s4", "s5", "s6", "s7"))
  make_all_dummies(df = df_all, to_factor = to_factor)
}

df_dummies_factors <- make_example(to_factor = TRUE)
df_dummies_chrs <- make_example(to_factor = FALSE)

print_dummy_variables <- function(df) {
  print(predict(caret::dummyVars(~ ., data = df, sep = "..", fullRank = FALSE), df))
  print(makedummies::makedummies(df, basal_level = TRUE))
}

df_chrs <- tibble(x1 = c("p", "q", "r"))
df_factors <- dplyr::mutate_all(df_chrs, factor)
print_dummy_variables(df_factors)
print_dummy_variables(df_chrs)
