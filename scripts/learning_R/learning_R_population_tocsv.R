library(assertthat)
library(dplyr)
library(readxl)
library(stringr)

population_tocsv <- function(in_filename, out_filename) {
    all_df <- readxl::read_excel(in_filename)
    all_polulation <- all_df[4,6]

    ## 都道府県、市区町村、人口、世帯数、転入、出生、転出、死亡、自然増減率、社会増減率を取り出す
    all_df <- all_df[5:NROW(all_df), c(2,3,6,7,10,11,16,17,23,25)]
    colnames(all_df) <- c('prefecture', 'city', 'population', 'households',
                          'in_migrants', 'live_births', 'out_migrants', 'deaths',
                          'natural_change_rate', 'net_migration_rate')

    ## 人口は整数に限る
    sapply(3:8, function(i) {
        all_df[[i]] <<- as.integer(all_df[[i]])
        i
    })

    ## 都道府県だけ取り出す
    pref_df <- all_df %>%
        dplyr::filter(is.na(city)) %>%
        dplyr::select(c(prefecture, population))
    assertthat::assert_that(NROW(pref_df)==47)

    ## 政令指定都市-区、群-町村の両階層が混じっているので、
    ## 市町村と東京特別区だけ取り出す
    ## 東京23区はそれぞれ別に数え、一まとめにしない
    city_df <- all_df
    city_df$city <- city_df$city %>%
        stringr::str_extract('^((.+(市|町|村))|([^市]+区))$')
    city_df <- city_df %>%
        dplyr::filter(!is.na(city))

    ## 市区町村を重複して数えていないか検算する
    pref_city_df <- city_df %>%
        dplyr::select(c(prefecture, population)) %>%
        dplyr::group_by(prefecture) %>%
        dplyr::summarize_each(sum) %>%
        dplyr::ungroup() %>%
        dplyr::rename(city_sum=population)

    checked_df <- dplyr::inner_join(pref_df, pref_city_df, by='prefecture') %>%
        dplyr::mutate(diff=population-city_sum)

    ## 都道府県人口と、都道府県の市区町村の人口の和の、差が0である
    assertthat::assert_that(all(checked_df$diff == 0))
    assertthat::assert_that(sum(checked_df$population)==all_polulation)

                                        # 市区町村名の先頭に都道府県名をつける
    city_df <- city_df %>%
        dplyr::mutate(city=paste0(prefecture, city)) %>%
        dplyr::filter(population > 0)

    readr::write_excel_csv(city_df, out_filename)
}

## 出典 data source : 総務省
## 【総計】平成30年住民基本台帳人口・世帯数、平成29年人口動態（市区町村別）
## https://www.soumu.go.jp/menu_news/s-news/01gyosei02_02000177.html
## https://www.soumu.go.jp/main_content/000563140.xls
g_in_filename <- 'incoming/000563140.xls'
g_out_filename <- 'population.csv'

population_tocsv(in_filename=g_in_filename, out_filename=g_out_filename)
