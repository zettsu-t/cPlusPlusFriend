library(plyr)
library(dplyr)
library(ggplot2)
library(gtable)
library(grid)
library(lubridate)
library(plotrix)
library(scales)

g_input_dir <- './data'
g_output_dir <- './images'
g_min_precipitation <- 0.5

g_font_size <- 24
g_annotation_size <- 11

## 出典: 気象庁 過去の気象データ・ダウンロード
## のヘッダ行を"date,temperature,precipitation"に取り替える
## 高知県本山町は、早明浦ダムの所在地
## https://www.data.jma.go.jp/gmd/risk/obsdl/index.php
g_csv_filename <- 'motoyama_weather.csv'

convert_date_string <- function(s) {
    ## すでに日本時間なので、tzを設定すると9時間ずれてしまう
    as.Date(lubridate::parse_date_time(s, orders='Y!m!*d!', locale='English'))
}

set_font_size <- function(g) {
    g + theme(axis.text=element_text(size=g_font_size),
              axis.title=element_text(size=g_font_size),
              strip.text=element_text(size=g_font_size),
              plot.title=element_text(size=g_font_size))
}

make_png_filename <- function(prefix, suffix) {
    file.path(g_output_dir,
              paste0(
                  stringr::str_replace_all(
                               paste0(prefix, suffix), '[^[:alnum:]]+', '_'), '.png'))
}

## 読み込みと前処理
df <- readr::read_csv(file.path(g_input_dir, g_csv_filename),
                      locale=readr::locale(encoding='UTF-8')) %>%
    dplyr::mutate(date=convert_date_string(date))
dir.create(g_output_dir)

## 描画
draw_with_grid <- function(df, log_precipitation) {
    filename_suffix <- ifelse(log_precipitation, '_log', '')

    ## 降水量無しは描かない
    df$precipitation[df$precipitation < g_min_precipitation] <- NA

    ## 浮動小数を、 ==max(values) で比較してはいけない
    df_max_precipitation <- df %>% top_n(1, precipitation)
    top_label <- paste0(df_max_precipitation$precipitation, 'mm')

    if (log_precipitation) {
        df$precipitation <- log10(df$precipitation)
    }

    ## この刻みは目分量
    precipitation.labels <- c(g_min_precipitation, 1, 10, 100, 500)
    if (log_precipitation) {
        precipitation.breaks <- log10(precipitation.labels)
    }

    if (!log_precipitation) {
        ## 3ヵ月おきにtickを打つ
        day_positions <- c(which((lubridate::day(df$date) == 1) &
                                 (lubridate::month(df$date) %in% c(1, 4, 7, 10))),
                           NROW(df$date))
        day_labs <- df$date[day_positions]
        day_all <- 1:NROW(df)

        png_filename <- make_png_filename('twoord', '')
        png(filename=png_filename, width=1024, height=640)
        twoord.plot(day_all, df$temperature,
                    day_all, df$precipitation,
                    main='Draw with twoord.plot',
                    xlab='Date', ylab='temperature [\u00B0C]', rylab='precipitation [mm]',
                    type=c('l', 'bar'), lwd=3, axislab.cex=2,
                    lcol=c('orange', 'black'), rcol=c('royalblue', 'black'),
                    xtickpos=day_positions, xticklab=day_labs)
        dev.off()
    }

    ## https://rpubs.com/kohske/dual_axis_in_ggplot2
    title <- 'Draw with ggplot2 + grid'
    png_filename <- make_png_filename('grid', filename_suffix)
    png(filename=png_filename, width=1024, height=640)
    p1 <- ggplot(df, mapping=aes(x=date, y=temperature))
    p1 <- p1 + geom_line(stat='identity', color='orange', size=2)
    p1 <- p1 + theme_bw()
    p1 <- p1 + scale_x_date(name='Date', labels=date_format('%Y/%m/%d'))
    p1 <- p1 + ggtitle(title)
    p1 <- p1 + ylab('temperature [\u00B0C]')
    p1 <- set_font_size(p1)

    p2 <- ggplot(df, mapping=aes(x=date, y=precipitation))
    ## theme_bw() %+replace% が無いとp1が描かれない
    p2 <- p2 + geom_bar(stat='identity', color='royalblue', fill='royalblue')
    p2 <- p2 + theme_bw() %+replace%
        theme(panel.background = element_rect(fill = NA),
              axis.title.y = element_text(color = 'grey'))
    p2 <- set_font_size(p2)

    if (log_precipitation) {
        p2 <- p2 + scale_y_continuous(name='precipitation [mm]',
                                      breaks=precipitation.breaks, labels=precipitation.labels)
    } else {
        p2 <- p2 + scale_y_continuous(sec.axis = sec_axis(~., name='precipitation [mm]'))
        p2 <- p2 + annotate('text', x=df_max_precipitation$date,
                            y=df_max_precipitation$precipitation - 20,
                            label=top_label, size=g_annotation_size)
    }

    grid.newpage()
    g1 <- ggplot_gtable(ggplot_build(p1))
    g2 <- ggplot_gtable(ggplot_build(p2))

    pp <- c(subset(g1$layout, name == 'panel', se = t:r))
    g <- gtable_add_grob(g1, g2$grobs[[which(g2$layout$name == 'panel')]], pp$t, pp$l, pp$b, pp$l)

    ## これが無いと右側の軸が描かれない
    ia <- which(g2$layout$name == 'axis-l')
    ga <- g2$grobs[[ia]]
    ax <- ga$children[[2]]
    ax$widths <- rev(ax$widths)
    ax$grobs <- rev(ax$grobs)
    ax$grobs[[1]]$x <- ax$grobs[[1]]$x - unit(1, 'npc') + unit(0.15, 'cm')
    g <- gtable_add_cols(g, g2$widths[g2$layout[ia, ]$l], length(g$widths) - 1)
    g <- gtable_add_grob(g, ax, pp$t, length(g$widths) - 1, pp$b)
    grid.draw(g)
    dev.off()


    ## https://shumagit.github.io/myblog/2018/09/02/ggplot2-sec-axis/
    precipitation.range <- range(na.omit(df$precipitation))
    temperature.range <- range(df$temperature)
    df$precipitation.scaled <- scales::rescale(df$precipitation, to=temperature.range)
    precipitation.scaled.range <- range(na.omit(df$precipitation.scaled))
    if (log_precipitation) {
        precipitation.breaks <- log10(precipitation.labels)
    } else {
        precipitation.breaks <- precipitation.labels
    }
    df_max_precipitation <- df %>% top_n(1, precipitation)

    ## https://stackoverflow.com/questions/3099219/ggplot-with-2-y-axes-on-each-side-and-different-scales
    title <- 'Draw with ggplot2 + sec_axis'
    png_filename <- make_png_filename('sec_axis', filename_suffix)
    png(filename=png_filename, width=1024, height=640)
    g <- ggplot()
    g <- g + geom_line(df, mapping=aes(x=date, y=temperature),
                       stat='identity', color='orange', size=2)
    g <- g + geom_bar(df, mapping=aes(x=date, y=precipitation.scaled),
                      stat='identity', color='royalblue')
    g <- g + scale_x_date(name='Date', labels=date_format('%Y/%b/%d'))
    g <- g + annotate('text', x=df_max_precipitation$date,
                      y=df_max_precipitation$precipitation.scaled - 2,
                      label=top_label, size=g_annotation_size)
    g <- g + scale_y_continuous(name='temperature [\u00B0C]',
                                sec.axis = sec_axis(
                                    ~scales::rescale(., to=precipitation.range),
                                    name='precipitation [mm]',
                                    breaks=precipitation.breaks, labels=precipitation.labels))
    g <- g + ggtitle(title)
    g <- set_font_size(g)
    plot(g)
    dev.off()
}

draw_with_grid(df, FALSE)
draw_with_grid(df, TRUE)
