get_stage_info <- function(sc) {
  count_column_elements <- function(xs) {
    suppressWarnings(max(max(purrr::map_int(xs, ~ NROW(.x))), 0))
  }

  sc_app_id <- sc %>%
    spark_context() %>%
    sparklyr::invoke("applicationId")

  sc_url <- sc %>%
    spark_context() %>%
    sparklyr::invoke("uiWebUrl") %>%
    sparklyr::invoke("get")

  # Numbers that int32 cannot represent become floating point numbers
  df_stages <- httr::GET(paste(sc_url, "api", "v1", "applications", sc_app_id, "stages", sep = "/")) %>%
    httr::content(as = "text") %>%
    fromJSON()

  if (NROW(df_stages) == 0) {
    return(NULL)
  }

  scalar_col_names <- df_stages %>%
    summarize_all(count_column_elements) %>%
    pivot_longer(everything()) %>%
    filter(value == 1) %>%
    pull(name)

  time_col_names <- scalar_col_names %>%
    keep(~ stringr::str_ends(.x, "Time"))

  other_scalar_col_names <- setdiff(scalar_col_names, time_col_names)

  purrr::reduce(.x = time_col_names, .init = df_stages, .f = function(acc, col_name) {
    values <- purrr::map(acc[[col_name]], ~ lubridate::as_datetime(unlist(.x)))
    if (min(na.omit(purrr::map_dbl(values, ~ lubridate::year(.x)))) > 1970) {
      # Assumes this column contains dates
      acc %>%
        mutate(!!rlang::sym(col_name) := values)
    } else {
      # Assumes this column contains integers
      acc
    }
  }) %>%
    tidyr::unnest(cols = all_of(other_scalar_col_names)) %>%
    arrange(stageId)
}

get_stage_id <- function(sc) {
  df_stages <- get_stage_info(sc)
  if (is.null(df_stages)) {
    NA
  } else {
    max(df_stages$stageId)
  }
}

plot_stage_info <- function(df, name, y_label, unit) {
  df_drawn <- if (is.null(unit) | is.na(unit)) {
    df %>%
      rename(y := !!rlang::sym(name))
  } else {
    df %>%
      mutate(y := !!rlang::sym(name) / unit)
  }

  g <- ggplot(df_drawn)
  g <- g + geom_line(aes(x = stageId, y = y), color = "royalblue")
  g <- g + ylab(y_label)
  g
}

get_r_mem <- function(x = NULL) {
  gc(reset = TRUE)
  df_mem <- as_tibble(gc(reset = TRUE))
  total_name <- paste0("total ", colnames(df_mem)[2])
  total_size <- sum(df_mem %>% dplyr::pull(2))

  obj_size <- if (is.null(x)) {
    NA
  } else {
    max(possibly(~ pryr::compare_size(x), otherwise = NA)())
  }
  list(total_name = total_name, total_size = total_size, obj_size = obj_size)
}
