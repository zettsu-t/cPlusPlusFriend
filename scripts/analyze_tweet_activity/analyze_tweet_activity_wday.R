library(extrafont)
library(plyr)
library(dplyr)
library(ggplot2)
library(lubridate)
library(purrrlyr)
library(readr)
library(readxl)
library(stringr)
library(tibble)

g_input_dir <- './data'
g_output_dir <- './images'
font_name <- 'Migu 1M'

## Do not change the system locale
convert_date_tz_string <- function(s) {
    lubridate::parse_date_time(s, orders='ab!d!H!M!S!z!*Y!', locale='English', tz='Asia/Tokyo')
}

convert_date_string <- function(s) {
    lubridate::parse_date_time(s, orders='Y!m!*d!H!M!S!', locale='English', tz='Asia/Tokyo')
}

set_colors <- function(g) {
    g <- g + scale_color_manual(name = "Wday",
                                labels = c('Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'),
                                values=c('hotpink4', rep('slateblue4', 5), 'hotpink4'))
    g <- g + scale_shape_manual(name = "Wday",
                                labels = c('Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'),
                                values=c(16, 2, 5, 6, 3, 4, 17))
    g
}

set_hour_ticks <- function(g) {
    g <- g + xlab('Hour (0:00 to 24:00)')
    g <- g + scale_x_continuous(breaks=seq(0,24.5,2))
    g
}

set_legend <- function(g, chart_title) {
    font_size <- 24
    g <- g + ggtitle(chart_title)
    g <- g + guides(color = guide_legend(ncol=7))
    g <- g + theme(text=element_text(family=font_name),
                   legend.position="top",
                   legend.text=element_text(size=20),
                   legend.title=element_blank(),
                   axis.text=element_text(family=font_name, size=24),
                   axis.title=element_text(family=font_name, size=24),
                   strip.text=element_text(family=font_name, size=24),
                   plot.title=element_text(family=font_name, size=24))
    g
}

make_output_filename <- function(out_basename, suffix, extension) {
    file.path(g_output_dir, paste0(out_basename, '_', suffix, '.', extension))
}

make_png_filename <- function(out_basename, suffix) {
    make_output_filename(out_basename, suffix, 'png')
}

make_csv_filename <- function(out_basename, suffix) {
    make_output_filename(out_basename, suffix, 'csv')
}

draw_all <- function(account) {
    in_filename <- account$in_filename
    account_name <- account$account_name
    first_date <- account$first_date
    last_date <- account$last_date

    dir.create(g_output_dir)
    out_basename <- in_filename
    matched <- stringr::str_match(in_filename, "(.*)\\.")
    if (!is.na(matched)) {
        out_basename <- matched[1,2]
    }

    df_base <- read_excel(file.path(g_input_dir, in_filename), sheet='tweets')
    data_ranged <- !is.na(first_date) && !is.na(last_date)

    ## Exclude replies
    df <- df_base %>%
        dplyr::filter(!grepl('^@.*', Text)) %>%
        plyr::rename(replace=c('Created At' = 'date_str')) %>%
        dplyr::mutate(date=convert_date_string(date_str))

    df$date <- convert_date_tz_string(df$date_str)
    df$hour <- lubridate::hour(df$date) + lubridate::minute(df$date) / 60.0
    df$wday <- as.character(lubridate::wday(df$date))

    df_sub <- df
    chart_title <- account_name
    if (data_ranged) {
        df_sub <- df %>% filter(between(date,
                                        convert_date_string(paste(first_date, '00:00:00')),
                                        convert_date_string(paste(last_date, '23:59:59'))))
        chart_title <- paste0(account_name, ' ', first_date, '-', last_date)
    }
    readr::write_csv(df_sub, make_csv_filename(out_basename, 'out'))

    df_log_likes <- tibble(Favorites=df_sub$Favorites, account=account_name)

    png(filename=make_png_filename(out_basename, 'like'), width=800, height=400)
    g <- ggplot(df_sub, aes(x=hour, y=Favorites, shape=wday, color=wday))
    g <- g + geom_point(size=4, alpha=0.9)
    g <- g + scale_y_log10()
    g <- g + ylab('いいね')
    g <- set_colors(g)
    g <- set_hour_ticks(g)
    g <- set_legend(g, chart_title)
    plot(g)
    dev.off()

    png(filename=make_png_filename(out_basename, 'rt'), width=800, height=400)
    g <- ggplot(df_sub, aes(x=hour, y=Retweets, shape=wday, color=wday))
    g <- g + geom_point(size=4, alpha=0.9)
    g <- g + scale_y_log10()
    g <- g + ylab('リツイート')
    g <- set_colors(g)
    g <- set_hour_ticks(g)
    g <- set_legend(g, chart_title)
    plot(g)
    dev.off()

    df_like_rt <- df %>% filter(Favorites > 0 & Retweets > 0)
    png(filename=make_png_filename(out_basename, 'like_rt'), width=800, height=400)
    g <- ggplot(df_like_rt, aes(x=Favorites, y=Retweets, shape=wday, color=wday))
    g <- g + xlab('いいね')
    g <- g + ylab('リツイート')
    g <- g + geom_point(size=4, alpha=0.9)
    g <- g + scale_x_log10()
    g <- g + scale_y_log10()
    g <- set_colors(g)
    g <- set_legend(g, chart_title)
    plot(g)
    dev.off()

    df_log_likes
}

## draw_all_files<- function(in_filename, account_name, first_date, last_date) {
draw_all_acoounts <- function(account_set) {
    df <- account_set %>%
        purrrlyr::by_row(function(x) { draw_all(x) }, .collate = c("list"))
    df <- df[['.out']] %>% plyr::ldply() %>% dplyr::bind_rows()

    png_filename <- file.path(g_output_dir, 'dist_all.png')
    png(filename=png_filename, width=800, height=400)
    g <- ggplot(df, aes(x=Favorites, color=account))
    g <- g + geom_line(stat='density', size=1.5)
    g <- g + scale_x_log10()
    g <- set_legend(g, 'いいねの分布')
    g <- g + xlab('いいね')
    plot(g)
    dev.off()
}

## Download an xlsx file from Vicinitas to analyze
## https://www.vicinitas.io/

## This script reads a CSV file which consists of
## column names: in_filename, account_name, first_date, last_date
## rows for each account: downloaded xlsx filename, label for charts, first date of tweets (or NA), last date of tweets (or NA)
args <- commandArgs(trailingOnly=TRUE)
list_filename <- if (length(args) >= 1) {
    args[1]
} else {
    'files.csv'
}
account_set <- readr::read_csv(file.path(g_input_dir, list_filename), locale=readr::locale(encoding="UTF-8"))
draw_all_acoounts(account_set)
