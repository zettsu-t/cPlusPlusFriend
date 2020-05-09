library(assertthat)
library(dplyr)
library(e1071)
library(exactRankTests)
library(ggplot2)
library(purrr)
library(readr)

## Data Source : 平成27年国勢調査 (UTF-8変換済)
## https://www.e-stat.go.jp/stat-search/files?page=1&layout=datalist&toukei=00200521&tstat=000001080615&cycle=0&tclass1=000001080697&tclass2=000001080698
df_base <- readr::read_csv('incoming/001_00.csv', skip=9)

## 都道府県だけ取り出す
df_pref <- df_base %>%
    dplyr::filter(地域識別コード=='a') %>%
    dplyr::filter(地域コード!='00000') %>%
    dplyr::select(c(5,6,11))

assertthat::assert_that(NROW(df_pref) == 47)
names(df_pref) <-c('pref', 'population', 'density')
df_pref <- df_pref %>%
    dplyr::filter(population > 0) %>%
    dplyr::mutate(population=log10(population), density=log10(density))

df_pref_sub <- df_pref %>%
    dplyr::arrange(population) %>%
    dplyr::top_n(-38, population)

## 市区町村だけ取り出す
df_city <- df_base %>%
    dplyr::filter(地域識別コード %in% c('1', '2', '3')) %>%
    dplyr::filter(地域コード!='00000') %>%
    dplyr::select(c(5,6,11))

names(df_city) <-c('city', 'population', 'density')

## 青ヶ島村より人口が少ない場合は除外する
df_city <- df_city %>%
    dplyr::filter(population > 100) %>%
    dplyr::mutate(population=log10(population), density=log10(density)) %>%
    dplyr::arrange(density)

svm_model_pref <- e1071::svm(density ~ population , df_pref)
df_pref$predicted <- as.vector(predict(svm_model_pref, df_pref))

svm_model_pref_sub <- e1071::svm(density ~ population , df_pref_sub)
df_pref_sub$predicted <- as.vector(predict(svm_model_pref, df_pref_sub))

svm_model_city <- e1071::svm(density ~ population , df_city)
df_city$predicted <- as.vector(predict(svm_model_city, df_city))

draw_chart <- function(input_df, label_font_size, png_filename) {
    x_name <- setdiff(names(input_df), c('population', 'density'))[1]
    xs <- na.omit(input_df$population)
    ys <- na.omit(input_df$density)
    cor_value <- cor(xs, ys)
    cor_x <- median(xs)
    cor_y <- max(ys) - 1
    x_range <- ceiling(range(xs) * 4) / 4.0
    x_breaks <- seq(from=x_range[1] - 0.5, to=x_range[2] + 0.5, by=0.5)

    font_name <- 'Migu 1M'
    font_size <- 24

    if (!is.na(png_filename)) {
        png(png_filename, width = 1600, height = 900)
    }
    g <- ggplot(input_df, aes(x=population, y=density, label=!!(rlang::sym(x_name))))
    g <- g + geom_label(alpha=0.6, family=font_name, size=label_font_size)
    g <- g + scale_x_continuous(breaks=x_breaks)
    g <- g + geom_smooth(method='lm', formula=y~x, size=2)

    suffix <- 'blue=LM'
    if ('predicted' %in% names(input_df)) {
        g <- g + geom_line(aes(x=population, y=predicted), color='orchid', size=2)
        suffix <- 'blue=LM, orchid=SVR'
    }

    g <- g + labs(title=paste(x_name, sprintf(": cor=%.3f", cor_value), suffix),
                  x='log10 (population)', y='log10 (population/area)')
    g <- g + theme(text=element_text(family=font_name),
                   legend.position="top",
                   legend.text=element_text(size=font_size),
                   legend.title=element_blank(),
                   axis.text=element_text(family=font_name, size=font_size),
                   axis.title=element_text(family=font_name, size=font_size),
                   strip.text=element_text(family=font_name, size=font_size),
                   plot.title=element_text(family=font_name, size=font_size))
    plot(g)
    if (!is.na(png_filename)) {
        dev.off()
    }

    print(wilcox.exact(x=input_df$population, y=input_df$density, paired=TRUE))
    print(wilcox.exact(x=input_df$population, y=input_df$density, paired=TRUE, conf.int=TRUE, conf.level=0.95))
    invisible(0)
}

draw_random_chart <- function(png_filename) {
    df <- tibble(x=rnorm(47, 0, 1), y=rnorm(47, 0, 1))

    if (!is.na(png_filename)) {
        png(png_filename, width = 640, height = 400)
    }
    g <- ggplot(df, aes(x=x, y=y))
    g <- g + geom_point()
    g <- g + geom_smooth(method='lm', formula=y~x)
    g <- g + geom_text(x=mean(df$x), y=mean(df$y), label=paste0("cor=", cor(df$x, df$y)))
    plot(g)

    if (!is.na(png_filename)) {
        dev.off()
    }

    print(wilcox.exact(x=df$x, y=df$y, paired=TRUE))
    invisible(0)
}

draw_random_chart('random.png')
draw_chart(input_df=df_pref, label_font_size=16, png_filename='pref_all.png')
draw_chart(input_df=df_city, label_font_size=10, png_filename='city.png')
draw_chart(input_df=df_pref_sub, label_font_size=16, png_filename='pref_sub.png')
