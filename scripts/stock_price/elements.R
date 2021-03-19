library(tidyverse)
library(assertthat)

## Download the periodic table of elements from
## https://gist.github.com/GoodmanSciences/c2dd862cd38f21b0ad36b8f96b4bf1ee

df_elements <- readr::read_csv("incoming/Periodic_Table_of_Elements.csv") %>%
  dplyr::distinct(AtomicNumber, .keep_all = TRUE) %>%
  dplyr::arrange(AtomicNumber) %>%
  dplyr::mutate(n_letters = stringr::str_length(Symbol)) %>%
  dplyr::mutate(first = stringr::str_to_lower(stringr::str_sub(Symbol, 1, 1))) %>%
  dplyr::mutate(last = stringr::str_to_lower(stringr::str_sub(Symbol, -1, -1)))

df_nodes <- purrr::reduce(
  .x = df_elements$AtomicNumber, .init = NULL,
  .f = function(acc, from_num) {
    df_sub <- df_elements %>%
      dplyr::filter(AtomicNumber == from_num)
    assertthat::are_equal(NROW(df_sub), 1)

    node_last <- df_sub[[1, "last"]]
    to_nums <- setdiff(
      (df_elements %>%
        dplyr::filter(first == node_last))$AtomicNumber,
      from_num
    )

    if (NROW(to_nums) == 0) {
      acc
    } else {
      df_sub <- tibble(from = from_num, to = to_nums)
      if (is.null(acc)) {
        df_sub
      } else {
        dplyr::bind_rows(acc, df_sub)
      }
    }
  }
)

df_from <- df_elements %>%
  dplyr::select("AtomicNumber", "Symbol") %>%
  dplyr::rename(from = AtomicNumber, from_Sym = Symbol)

df_to <- df_elements %>%
  dplyr::select("AtomicNumber", "Symbol", "n_letters") %>%
  dplyr::rename(to = AtomicNumber, to_Sym = Symbol)

df_nodes <- dplyr::inner_join(dplyr::inner_join(df_nodes, df_from), df_to)

df_nodes <- dplyr::inner_join(df_nodes,
  df_elements %>% dplyr::select("AtomicNumber", "Element", "Symbol"),
  by = c("from_Sym" = "Symbol")
)

assertthat::assert_that(all(df_nodes$from == df_nodes$AtomicNumber))
assertthat::assert_that(all(df_nodes$n_letters == stringr::str_length(df_nodes$to_Sym)))
g_df_result <- tibble()

find_longest_path <- function(in_df, node_from, nodes_visited) {
  all_nodes <- c(nodes_visited, node_from)
  df_nodes_to <- in_df %>%
    dplyr::filter((from == node_from) & !(to %in% all_nodes)) %>%
    dplyr::sample_n(NROW(.)) %>%
    dplyr::arrange(n_letters)

  if (NROW(df_nodes_to) == 0) {
    assertthat::are_equal(NROW(all_nodes), NROW(unique(all_nodes)))
    df_result <- dplyr::inner_join(tibble(from = all_nodes), df_elements,
      by = c("from" = "AtomicNumber")
    ) %>%
      dplyr::select(c("Element", "from", "Symbol", "first", "last")) %>%
      dplyr::distinct(.keep_all = FALSE)

    if (NROW(df_result) > 73) {
      print(df_result$from)
    }

    if (NROW(g_df_result) < NROW(df_result)) {
      g_df_result <<- df_result
      readr::write_csv(df_result, "chain.csv")
    }

    df_result
  } else {
    df_next <- in_df %>%
      dplyr::filter(!(from %in% all_nodes) & !(to %in% all_nodes))

    purrr::reduce(.x = df_nodes_to$to, .init = tibble(), .f = function(acc, node_to) {
      df_result <- find_longest_path(df_next, node_to, all_nodes)
      if (NROW(acc) < NROW(df_result)) {
        df_result
      } else {
        acc
      }
    })
  }
}

find_longest_path(df_nodes, sample(df_elements$AtomicNumber, 1), c())
