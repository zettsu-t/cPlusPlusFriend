library(dplyr)
library(forcats)
library(stringr)
library(tibble)
library(tidyr)

xs <- c('foo2', 'foo10', 'foo1')
(as.data.frame(stringr::str_match(xs, '(.*\\D)(\\d+)$'), stringsAsFactors=FALSE) %>% dplyr::mutate_at(3, as.numeric) %>% dplyr::arrange(.[[3]]))[,1]

## Ruby
## xs.sort_by { |v| v.match(/(\d+)\z/)[1].to_i }

xs <- c('foo2.1ab6.5', 'foo10db', 'foo1', 'foo2.1ab4.3', 'foo10da')

sort_numbers <- function(xs_df) {
    if (NROW(xs_df) <= 1) {
        xs_df
    } else {
        df <- as_tibble(stringr::str_match(xs_df$suffix, '((\\d*[.])?\\d+)(.*)'))
        names(df) <- paste0('V', seq_len(NCOL(df)))
        df$original <- xs_df$original
        df %>%
            dplyr::rename(key=V2, suffix=V4) %>%
            dplyr::mutate(n=as.numeric(key)) %>%
            dplyr::arrange(n) %>%
            dplyr::select(c('n', 'suffix', 'original')) %>%
            dplyr::rename(key='n') %>%
            dplyr::group_by(key) %>%
            do(sort_strings(.)) %>%
            dplyr::ungroup() %>%
            dplyr::select(c('original'))
    }
}

sort_strings <- function(xs_df) {
    if (NROW(xs_df) <= 1) {
        xs_df
    } else {
        df <- as_tibble(stringr::str_match(xs_df$suffix, '([.\\D]*)(.*)'))
        names(df) <- paste0('V', seq_len(NCOL(df)))
        df$original <- xs_df$original
        df %>%
            dplyr::rename(key=V2, suffix=V3) %>%
            dplyr::arrange(key) %>%
            dplyr::select(c('key', 'suffix', 'original')) %>%
            dplyr::group_by(key) %>%
            do(sort_numbers(.)) %>%
            dplyr::ungroup() %>%
            dplyr::select(c('original'))
    }
}

sort_by_alnums <- function(xs) {
  suppressWarnings(stringr::str_match_all(xs, "(\\d+_?|\\D+_?)") %>%
    purrr::map(~ as_tibble(t(.x[,1, drop=FALSE]))) %>%
      dplyr::bind_rows() %>%
      dplyr::mutate_if(functional::Compose(as.numeric, is.na, `!`, all), as.numeric) %>%
      dplyr::mutate(original = xs) %>%
      dplyr::arrange_all())
}

sort_strings(tibble(suffix=xs, original=xs))
sort_by_alnums(c('foo2', 'foo10', 'foo1'))
sort_by_alnums(c('r_1_z', 'r_10_y', 'r_21_x', 'r_2_w', 'r_1_v'))

df_base_a <- tibble(key=c('r_1', 'r_10', 'r_21', 'r_2'))
df_a <- df_base_a %>%
    tidyr::separate(remove=FALSE, col='key', into=c('prefix', 'suffix')) %>%
    dplyr::arrange(as.integer(suffix))
forcats::fct_relevel(as.factor(df_base_a$key), df_a$key)

## https://stackoverflow.com/questions/11232801/regex-split-numbers-and-letter-groups-without-spaces
df_base_b <- tibble(key=c('r1', 'r10', 'r21', 'r2'))
df_b <- df_base_b %>%
    tidyr::separate(remove=FALSE, sep="(?<=\\D)(?=\\d)", col='key', into=c('prefix', 'suffix')) %>%
    dplyr::arrange(as.integer(suffix))
forcats::fct_relevel(as.factor(df_base_b$key), df_b$key)
