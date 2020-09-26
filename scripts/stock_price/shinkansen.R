## Windowsのバージョンとリリース年、の何番煎じか
library(tidyverse)
library(dplyr)
library(forcats)
library(ggplot2)
library(purrr)
library(stringr)
library(extrafont)

## 営業開始年(一年に複数ある場合は月も)の出典
## ドクターイエロー923形は除く
## 0系=東海道新幹線(1964)、200系=東北新幹線(1982)、400系=山形新幹線(1992/7)、
## 800系=九州新幹線(2004)の開業
##
## 100 (1985), 300 (1992/3), E1 (1994/7), 500 (1997/3), 700 (1999/3)
## https://museum.jr-central.co.jp/rolling-stock/
## https://company.jr-central.co.jp/company/about/history.html
## https://www.jreast.co.jp/Youran/pdf/2018-2019/jre_youran_shogen_p89-93.pdf
## https://www.westjr.co.jp/company/info/issue/data/pdf/data2019_10.pdf
##
## E2 (1997/3), E3 (1997/3), E4 (1997/12), E5 (2011/3), E6 (2013/3), E7 (2014/3)
## https://www.jreast.co.jp/train/shinkan/e4.html
##
## N700 (2007/7), N700A (2013/2), N700S (2020/7)
## https://railway.jr-central.co.jp/train/shinkansen/detail_01_01/index.html

label_set <- c('0系', '200系', '100系', '300系',
               '400系', 'E1系',
               '500系', 'E2系', 'E3系', 'E4系',
               '700系', '800系', 'N700系',
               'E5/H5系', 'N700A', 'E6系', 'E7/W7系', 'N700S')
label_set <- factor(label_set) %>% forcats::fct_relevel(label_set)

year_set <- c(1964, 1982, 1985, 1992.03,
              1992.07, 1994,
              1997.03, 1997.03, 1997.03, 1997.12,
              1999, 2004, 2007,
              2011, 2013.02, 2013.03, 2014, 2020)

color_set <- c('blue1', 'springgreen4', 'blue1', 'blue1',
               'gray50', 'hotpink2',
               'mediumpurple3', 'hotpink2', 'brown3', 'gold3',
               'blue1', 'red3', 'blue1',
               'springgreen3', 'blue1', 'red2', 'goldenrod3', 'blue2')

y_set <- purrr::map_int(label_set, function(x) {
    as.integer(stringr::str_match(x, '^\\D*(\\d+)')[1,2])
})

n_bars <- max(tabulate(year_set))
x_shift <- 1.0 / n_bars
width <- 1.0 / n_bars

df <- tibble(y=1+y_set, year=year_set, label=label_set)
df$x <- purrr::reduce(.x=year_set, .init=numeric(), .f=function(acc, x) {
    int_x <- floor(x)
    count <- NROW(purrr::keep(acc, ~ floor(.x) == int_x))
    c(acc, int_x + count * x_shift)
})

df$y_text <- df$y
## df$y_text <- purrr::map_dbl(1:NROW(df), function(i) {
##     df$y[i] * 1
## })

year_low_set <- sprintf('%02d', as.integer(floor(df$year %% 100)))
year_low_set[duplicated(year_low_set)] <- ''
df$year_low <- year_low_set

font_name <- 'Migu 1M'
font_size <- 14

png(filename='shinkansen_series.png', width=800, height=480)
g <- ggplot(df, aes(x=x, y=y, color=label, fill=label))
g <- g + geom_bar(stat='identity', position='identity', width=width, alpha=0.7)
g <- g + geom_text(aes(y=y_text, label=label), vjust=-0.2, color='black', size=5, family=font_name)
g <- g + scale_color_manual(values=color_set)
g <- g + scale_fill_manual(values=color_set)
g <- g + scale_y_log10()
g <- g + ggtitle('新幹線車両(営業運転開始年)')
g <- g + xlab('Year')
g <- g + ylab('Series')
g <- g + scale_x_continuous(breaks=df$x, labels=df$year_low)
g <- g + theme_bw()
g <- g + theme(text=element_text(family=font_name),
               legend.position='none',
               legend.text=element_text(size=font_size),
               legend.title=element_blank(),
               axis.ticks.x = element_blank(),
               axis.text=element_text(family=font_name, size=font_size),
               axis.title=element_text(family=font_name, size=font_size),
               strip.text=element_text(family=font_name, size=font_size),
               plot.title=element_text(family=font_name, size=font_size))
plot(g)
dev.off()
