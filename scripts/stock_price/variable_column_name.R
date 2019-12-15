library(assertthat)
library(dplyr)
library(readxl)
library(stringr)
library(tibble)
library(tidyr)

## 出典 data source : 総務省
## 【総計】平成30年住民基本台帳人口・世帯数、平成29年人口動態（市区町村別）
## https://www.soumu.go.jp/menu_news/s-news/01gyosei02_02000177.html
## https://www.soumu.go.jp/main_content/000563140.xls
## から抜粋

## 神奈川県の政令指定都市の人口を取り出す
in_filename <- 'city/incoming/000563140.xls'
all_df <- readxl::read_excel(in_filename)
all_df <- all_df[5:NROW(all_df), c(2,3,6)]
names(all_df) <- c('pref', 'city_ward', 'population')

all_df <- as_tibble(all_df)
all_df$population <- as.integer(all_df$population)

kanagawa_df <- all_df %>%
    dplyr::filter(pref=='神奈川県')

city_df <- kanagawa_df[!is.na(kanagawa_df$city_ward %>% stringr::str_extract('^.+市.+区$')),] %>%
    tidyr::separate('city_ward', into=c('city', 'ward'), sep='市') %>%
    dplyr::mutate(city=paste0(city, '市'))

## 横浜市、川崎市、相模原市の人口
expected <- c(3737845, 1488031, 718192)


## 列名をべた書きする(シンボル)
actual <- city_df %>% dplyr::select(-ward) %>%
    dplyr::group_by(pref, city) %>%
    dplyr::summarize_each(list(sum)) %>%
    ungroup()
assertthat::assert_that(assertthat::are_equal(expected, actual$population))
summary_df <- actual

## selectに文字列リテラルを渡す
actual <- city_df %>% dplyr::select(-'ward') %>%
    dplyr::group_by(pref, city) %>%
    dplyr::summarize_each(list(sum)) %>%
    ungroup()
assertthat::assert_that(assertthat::are_equal(expected, actual$population))

## selectに文字列変数を渡す。これができないとハードコーディングだらけで辛い。
name <- 'ward'
actual <- city_df %>% dplyr::select(-(!!name)) %>%
    dplyr::group_by(pref, city) %>%
    dplyr::summarize_each(list(sum)) %>%
    ungroup()
assertthat::assert_that(assertthat::are_equal(expected, actual$population))

name_set <- c('pref', 'ward')
actual <- city_df %>% dplyr::select(-(!!(name_set))) %>%
    dplyr::group_by(city) %>%
    dplyr::summarize_each(list(sum)) %>%
    ungroup()
assertthat::assert_that(assertthat::are_equal(expected, actual$population))


## group_byに文字列変数を渡す。これができないとハードコーディングだらけで辛い。
name <- c('city')
actual <- city_df %>% dplyr::select(-c('pref', 'ward')) %>%
    dplyr::group_by(!!(rlang::sym(name))) %>%
    dplyr::summarize_each(list(sum)) %>%
    ungroup()
assertthat::assert_that(assertthat::are_equal(expected, actual$population))

name_set <- c('pref', 'city')
actual <- city_df %>% dplyr::select(-'ward') %>%
    dplyr::group_by(!!!(rlang::syms(name_set))) %>%
    dplyr::summarize_each(list(sum)) %>%
    ungroup()
assertthat::assert_that(assertthat::are_equal(expected, actual$population))


## mutate先の列名をべた書きする(シンボル)
actual <- summary_df %>% dplyr::mutate(n=population)
assertthat::assert_that(assertthat::are_equal(expected, actual$population))

## mutate先の列名として文字列変数を渡す。これができないとハードコーディングだらけで辛い。
name <- c('n')
actual <- summary_df %>% dplyr::mutate(!!(name):=population)
assertthat::assert_that(assertthat::are_equal(expected, actual$n))

x_name <- c('population')
y_name <- c('y')
actual <- summary_df %>% dplyr::mutate(!!(y_name):=!!(rlang::sym(x_name)))
assertthat::assert_that(assertthat::are_equal(expected, actual$y))


## 指定した列から特定の要素を抜き出す
pattern <- '横浜市.+区'
n_wards <- 18

## 列名をハードコーディングする
actual <- na.omit(kanagawa_df$city_ward %>% str_extract(pattern))
assertthat::assert_that(n_wards == NROW(actual))

actual <- na.omit(kanagawa_df[['city_ward']] %>% str_extract(pattern))
assertthat::assert_that(n_wards == NROW(actual))

name <- 'city_ward'
actual <- na.omit(kanagawa_df[[name]] %>% str_extract(pattern))
assertthat::assert_that(n_wards == NROW(actual))
