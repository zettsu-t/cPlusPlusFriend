library(assertthat)
library(extrafont)
library(ineq)
library(dplyr)
library(readxl)
library(stringr)

## 任意のフォントを使うには、あらかじめ以下を実行しておく(数分かかる)
## install.packages('extrafont')
## font_import()
## windowsFonts() で、指定可能なフォントが分かる
g_font_name <- 'Migu 1M'

read_population_sheet <- function(in_filename) {
    ## 都道府県、市区町村、人口を取り出す
    all_df <- readxl::read_excel(in_filename)
    all_polulation <- all_df[4,6]
    all_df <- all_df[5:NROW(all_df), c(2,3,6)]
    names(all_df) <- c('pref', 'city', 'population')
    all_df$population <- as.integer(all_df$population)

    ## 都道府県だけ取り出す
    pref_df <- all_df %>%
        dplyr::filter(is.na(city)) %>%
        dplyr::select(c(pref, population))

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
        dplyr::select(-city) %>%
        dplyr::group_by(pref) %>%
        dplyr::summarize_each(sum) %>%
        dplyr::ungroup() %>%
        dplyr::rename(city_sum=population)

    checked_df <- dplyr::inner_join(pref_df, pref_city_df, by='pref') %>%
        dplyr::mutate(diff=population-city_sum)

    ## 都道府県人口と、都道府県の市区町村の人口の和の、差が0である
    assertthat::assert_that(all(checked_df$diff == 0))
    assertthat::assert_that(sum(checked_df$population)==all_polulation)

    ## 市区町村名の先頭に都道府県名をつける
    city_df <- city_df %>%
        dplyr::mutate(city=paste0(pref, city)) %>%
        dplyr::select(c(city, population))

    list(pref_df=pref_df, city_df=city_df)
}

## ジニ係数を求めてローレンツ曲線を描く
draw_lorenz_curve <- function(df, name) {
    print(head(df %>% dplyr::arrange(desc(population)), 10))
    gini_coef <- ineq::ineq(df$population,type="Gini")
    gini_coef_text <- sprintf('ジニ係数 %.3f', gini_coef)

    title <- paste0('ローレンツ曲線 (', name, '人口)')
    x_label <- paste0(name, 'の累積和')
    y_label <- paste0(name, 'の人口の累積和')

    plot(Lc(df$population), main=title, xlab=x_label, ylab=y_label, family=g_font_name)
    text(x=0.2, y=0.8, labels=gini_coef_text, cex=1.5, family=g_font_name)
    gini_coef
}

draw_all <- function(in_filename, out_filename) {
    df_set <- read_population_sheet(in_filename)
    png(out_filename, width = 800, height = 400)
    par(mfrow=c(1,2))
    draw_lorenz_curve(df=df_set$pref_df, name='都道府県')
    draw_lorenz_curve(df=df_set$city_df, name='市区町村')
    dev.off()
    df_set
}

## Data source : 総務省
## 【総計】平成30年住民基本台帳人口・世帯数、平成29年人口動態（市区町村別）
## https://www.soumu.go.jp/menu_news/s-news/01gyosei02_02000177.html
## https://www.soumu.go.jp/main_content/000563140.xls
in_filename <- 'incoming/000563140.xls'
out_filename <- 'population_gini_coef.png'
df_set <- draw_all(in_filename=in_filename, out_filename=out_filename)
