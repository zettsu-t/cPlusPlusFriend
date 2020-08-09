## Based on
## https://vietle.info/post/calendarheatmap/
library(tidyverse)
library(forcats)
library(ggplot2)
library(lubridate)
library(purrrlyr)
library(tibble)
library(tidyr)
library(assertthat)
library(zipangu)
library(extrafont)

daytype_set <- c('プログラマたん3周年', '祝日', '日曜', '土曜', '私の人生を集中工事', '平日')
color_set <- c('royalblue1', 'orchid2', 'plum2', 'lightblue2', 'wheat2', 'white')
month_df <- tibble(month=1:12,
                   name=c('睦月', '如月', '弥生', '卯月', '皐月', '水無月',
                          '文月', '葉月', '長月', '神無月', '霜月', '師走'))

month_labeller <- function(x) {
    dplyr::left_join(x, month_df) %>%
        dplyr::mutate(value=paste0(month, '月(', name, ')')) %>%
            dplyr::select(c('value'))
}

in_vacation <- function(arg_date) {
    ((ymd('2020/01/01') <= arg_date) & (arg_date <= ymd('2020/01/05'))) ||
    ((ymd('2020/04/29') <= arg_date) & (arg_date <= ymd('2020/05/06'))) ||
    ((ymd('2020/07/18') <= arg_date) & (arg_date <= ymd('2020/07/29')))
}

get_day_attr <- function(day_row) {
    value <- if ((day_row$month == 4) && (day_row$day == 25)) {
        'プログラマたん3周年'
    } else if (day_row$holiday) {
        '祝日'
    } else if (day_row$weekday == 1) {
        '日曜'
    } else if (day_row$weekday == 7) {
        '土曜'
    } else if (in_vacation(day_row$date_col)) {
        '私の人生を集中工事'
    } else {
        '平日'
    }
    assertthat::assert_that(value %in% daytype_set)
    value
}

date_set <- seq(ymd('2020/01/01'), ymd('2020/12/31'), 'days')
df <- tibble(date_col=date_set) %>%
    dplyr::mutate(weekday=lubridate::wday(date_col, label=FALSE, week_start=7),
                  weekday_label=lubridate::wday(date_col, label=TRUE, week_start=7),
                  month=lubridate::month(date_col, label=FALSE),
                  day=lubridate::day(date_col),
                  date=lubridate::yday(date_col),
                  week=lubridate::epiweek(date_col)) %>%
    dplyr::mutate(holiday=is_jholiday(date_set))

dfx <- purrrlyr::by_row(df, .collate = c('cols'), .to = 'daytype', function(x) {
    get_day_attr(x)
}) %>%
    dplyr::mutate(daytype=forcats::fct_relevel(daytype, daytype_set))

font_name <- 'Migu 1M'
png(filename='calendar2020.png', width=1024, height=640)
g <- ggplot(dfx, aes(weekday_label, -week, fill=daytype))
g <- g + geom_tile(color='black')
g <- g + geom_text(aes(label=day(date_col)), size=4)
g <- g + theme(text=element_text(family=font_name),
               aspect.ratio = 3/5,
               axis.title.x = element_blank(),
               axis.title.y = element_blank(),
               axis.text.y = element_blank(),
               panel.grid = element_blank(),
               axis.ticks = element_blank(),
               panel.background = element_blank(),
               strip.background = element_blank(),
               strip.text = element_text(size=12),
               legend.title=element_blank(),
               legend.position='right',
               legend.text=element_text(size=10))
g <- g + scale_fill_manual(values=color_set)
g <- g + facet_wrap(~month, nrow=4, ncol=3, scales='free', labeller=month_labeller)
g <- g + labs(title = '2020年カレンダー')
plot(g)
dev.off()
