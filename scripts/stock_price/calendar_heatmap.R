library(tidyverse)
library(forcats)
library(ggplot2)
library(ggpubr)
library(lubridate)
library(purrrlyr)
library(tibble)
library(tidyr)
library(readr)
library(assertthat)
library(zipangu)
library(extrafont)

## Data source
## https://markets.cboe.com/us/equities/market_statistics/historical_market_volume/

read_volume <- function(in_filename, market_name) {
    df <- readr::read_csv(in_filename) %>%
        dplyr::filter(`Market Participant` == market_name) %>%
        dplyr::select(c('Day', 'Total Shares')) %>%
        dplyr::rename(day=Day, volume='Total Shares') %>%
        dplyr::mutate(year=lubridate::year(day),
                      monthday=lubridate::day(day),
                      weekday=lubridate::wday(day, label=FALSE, week_start=1),
                      month=lubridate::month(day, label=FALSE),
                      week=lubridate::epiweek(day)) %>%
        dplyr::filter(weekday < 6) %>%
        dplyr::mutate(week=ifelse(month > 1 & week==1, max(week) + 1, week)) %>%
        dplyr::mutate(week_y=weekday-1) %>%
        dplyr::arrange(day)

    df <- dplyr::bind_rows(purrr::map(1:12, function(m) {
        df %>%
            dplyr::filter(month==m) %>%
            dplyr::mutate(month_first=c(TRUE, rep(FALSE, n() - 1)),
                          month_last=c(rep(FALSE, n() - 1), TRUE))
    }))

    ## Split days by monthes
    df_segment <- dplyr::bind_rows(purrr::map(1:11, function(m) {
        df_prev <- df %>%
            dplyr::filter(month==m & month_last==TRUE)
        df_next <- df %>%
            dplyr::filter(month==(m+1) & month_first==TRUE)

        if (df_prev$week == df_next$week) {
            tibble(x=   c(df_prev$week + 0.5, df_prev$week + 0.5, df_prev$week - 0.5),
                   xend=c(df_prev$week + 0.5, df_prev$week - 0.5, df_prev$week - 0.5),
                   y=   c(-0.5, df_prev$week_y + 0.5, df_prev$week_y + 0.5),
                   yend=c(df_prev$week_y + 0.5, df_prev$week_y + 0.5, 4.5))
        } else {
            tibble(x=df_prev$week + 0.5, xend=df_prev$week + 0.5, y=-0.5, yend=4.5)
        }
    }))

    list(df=df, df_segment=df_segment)
}

draw_volume <- function(df_set, year, market_name) {
    ## Based on
    ## https://dominikkoch.github.io/Calendar-Heatmap/
    ## https://stackoverflow.com/questions/27000131/calendar-heat-map-tetris-chart
    df <- df_set$df
    df_segment <- df_set$df_segment

    font_name <- 'Segoe UI'
    g <- ggplot(df)
    g <- g + geom_tile(aes(week, week_y, fill=volume), colour='white')
    g <- g + geom_text(aes(week, week_y, label=monthday), color='gray60', size=3)
    g <- g + coord_equal(xlim = c(2.5,53))
    g <- g + scale_x_continuous(breaks=54/12*(1:12)-2,
                                labels=c('Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'))
    g <- g + scale_y_continuous(breaks = 0:4, labels = c('Mon','Tue','Wed','Thu','Fri'))
    g <- g + scale_fill_gradient(low='azure', high='navy')
    g <- g + geom_segment(data=df_segment, aes(x=x, xend=xend, y=y, yend=yend), size=1)
    g <- g + labs(title=paste0(year, '年 出来高 : ', market_name), size=2)
    g <- g + theme(text=element_text(family=font_name),
                   axis.title.x = element_blank(),
                   axis.title.y = element_blank(),
                   panel.grid = element_blank(),
                   panel.background = element_blank(),
                   strip.background = element_blank(),
                   strip.text = element_text(size=12),
                   legend.title = element_text(size=12),
                   legend.position='right',
                   legend.text=element_text(size=10))
    g
}

draw_volume_years <- function(years, market_names) {
    purrr::reduce(.x=years, .init=list(), .f=function(acc, year) {
        gs <- purrr::map(.x=market_names, .f=function(market_name) {
            in_filename <- paste0('incoming/market_history_', year,'.csv')
            df_set <- read_volume(in_filename, market_name)
            draw_volume(df_set, year, market_name)
        })
        c(acc, gs)
    })
}

png(filename='stock_volume.png', width=1600, height=960)
gs <- draw_volume_years(years=c(2017, 2018, 2019), market_names=c('NYSE (N)', 'NASDAQ (Q)'))
g <- ggpubr::ggarrange(plotlist=gs, ncol=1)
plot(g)
dev.off()
