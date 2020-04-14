library(dplyr)
library(stringr)
library(tibble)

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

result <- sort_strings(tibble(suffix=xs, original=xs))
