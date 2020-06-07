library(dplyr)
library(extrafont)
library(ggplot2)
library(purrr)
library(readr)
library(readxl)
library(stringr)
library(tidyr)

read_data <- function() {
    ## 平成20～24年 人口動態保健所・市区町村別統計
    ## https://www.e-stat.go.jp/stat-search/files?page=1&query=%E5%90%88%E8%A8%88%E7%89%B9%E6%AE%8A%E5%87%BA%E7%94%9F%E7%8E%87&layout=dataset&stat_infid=000023620470
    df_birth <- readxl::read_xlsx('incoming/h20-24.xlsx', skip=6) %>%
        dplyr::select(c(1,2))
    colnames(df_birth) <- c('city_code', 'rate')
    df_birth$rate <- as.numeric(df_birth$rate)
    df_birth <- tidyr::separate(df_birth, 'city_code', c('prefix', 'code', 'city_birth')) %>%
        dplyr::filter((prefix == "") & (code != "") & (city_birth != "")) %>%
        dplyr::select(c('code', 'city_birth', 'rate')) %>%
        dplyr::mutate(city_birth=stringr::str_replace_all(city_birth, "[ 　]+", ""))

    ## 市区町村人口 平成22年国勢調査 (UTF-8変換済)
    ## https://www.e-stat.go.jp/stat-search/files?page=1&layout=datalist&toukei=00200521&tstat=000001039448&cycle=0&tclass1=000001045009&tclass2=000001046265
    df_population <- readr::read_csv('incoming/001_utf8.csv', skip=10) %>%
        dplyr::select(c(3,7,8))
    colnames(df_population) <- c('code', 'city_population', 'population')
    df_population <- df_population %>%
        dplyr::filter(!stringr::str_detect(city_population, "人口集中地区")) %>%
        dplyr::filter(!stringr::str_detect(city_population, "\\(旧")) %>%
        dplyr::mutate(city_population=stringr::str_replace_all(city_population, "[ 　]+", ""))

    df <- dplyr::inner_join(df_birth, df_population, by=c('code'))

    ## 市町村合併を除く
##  df <- purrrlyr::by_row(df, function(city_data) {
##      stringr::str_detect(city_data$city_population, city_data$city_birth)
##  }, .collate=c("cols"), .to = "same")

    dplyr::mutate(df, city=city_population)
}

draw_data <- function(df, draw_label, x_log, png_filename) {
    font_name <- 'Migu 1M'

    png(filename=png_filename, width=1200, height=800)
    g <- ggplot(df, aes(x=population, y=rate, label=city))
    if (draw_label) {
        g <- g + geom_text(family=font_name, size=3)
    } else {
        g <- g + geom_point()
    }
    if (x_log) {
        g <- g + scale_x_log10()
    }
    g <- g + labs(title='市区町村人口 平成22年国勢調査 * 平成20～24年 人口動態保健所・市区町村別統計',
                  x='市区町村人口 (東京23区と政令指定都市は各区)', y='合計特殊出生率')
    g <- g + theme(text=element_text(family=font_name),
                   legend.position="top",
                   legend.text=element_text(size=20),
                   legend.title=element_blank(),
                   axis.text=element_text(family=font_name, size=20),
                   axis.title=element_text(family=font_name, size=20),
                   strip.text=element_text(family=font_name, size=20),
                   plot.title=element_text(family=font_name, size=20))
    plot(g)
    dev.off()
}

df <- read_data()
draw_data(df=df, draw_label=TRUE, x_log=TRUE, png_filename='label_log.png')
draw_data(df=df, draw_label=TRUE, x_log=FALSE, png_filename='label_linear.png')
draw_data(df=df, draw_label=FALSE, x_log=TRUE, png_filename='point_log.png')
draw_data(df=df, draw_label=FALSE, x_log=FALSE, png_filename='point_linear.png')
