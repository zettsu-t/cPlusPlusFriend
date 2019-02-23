library(extrafont)
library(data.table)
library(ggplot2)
library(stringr)
library(plyr)
library(dplyr)

## Depart from Shinagawa at 11:18
## Data source : Keikyu Official Website
## https://www.keikyu.co.jp/train-info/index.html

df <- fread('incoming/timetable.csv', encoding='UTF-8')
df$index <- 1:NROW(df)

to_minutes <- function(time_str) {
    matched <- str_match(time_str, '(\\d+):(\\d+)')
    ifelse(is.na(matched[1,1]), NA,
           as.integer(matched[1,2]) * 60 + as.integer(matched[1,3]))
}

to_minutes_set <- function(time_str_set) {
    sapply(time_str_set, function(time_str) {
        to_minutes(time_str)
    })
}

df <- df %>%
    mutate(kaitoku_duration=to_minutes_set(kaitoku),
           express_duration=to_minutes_set(express),
           local_duration=to_minutes_set(local))

start_time <- min(df$kaitoku_duration, df$express_duration, df$local_duration, na.rm=TRUE)
kaitoku_stops_indexes <- which(sapply(df$kaitoku, function(x) { !is.na(x) && nchar(x) > 0 } ))
df$kaitoku_stops <- sapply(1:NROW(df), function(x) { ifelse(x %in% kaitoku_stops_indexes, x, NA) })

kaitoku_first_stop <- kaitoku_stops_indexes[2]
start_time <- start_time +
    c(0, rep(df$local_duration[1] - df$kaitoku_duration[1], kaitoku_first_stop - 2),
      rep(0, NROW(df) + 1 - kaitoku_first_stop))

df <- df %>%
    mutate(kaitoku_duration=kaitoku_duration - start_time,
           express_duration=express_duration - start_time,
           local_duration=local_duration - start_time)

df <- df %>% rowwise() %>%
    mutate(duration=min(kaitoku_duration, express_duration, local_duration, na.rm=TRUE))

font_name <- 'Migu 1M'
par(family=font_name)
png(filename='keikyu.png', width=800, height=480)
g <- ggplot(df)
g <- g + geom_line(aes(x=index, y=duration), color='navy', size=2)
g <- g + geom_line(aes(x=kaitoku_stops, y=kaitoku_duration), color='violetred3', size=1)
g <- g + ggtitle('品川からの所要時間(赤:快特、青:急行と普通に乗り換え)')
g <- g + ylab('品川からの所要時間[分]')
g <- g + scale_x_continuous(breaks=1:NROW(df), labels=df$station)
g <- g + geom_vline(xintercept=df$index[kaitoku_stops_indexes], color='royalblue', size=1)
g <- g + theme(text=element_text(family=font_name),
               legend.position=c(0.5, 0.5),
               panel.grid.major=element_line(color='grey50'),
               axis.text.x=element_text(family=font_name, color='black', size=10, angle=45, vjust=.8, hjust=0.8),
               axis.text.y=element_text(family=font_name, color='black', size=14),
               axis.title.x=element_blank(),
               axis.title.y=element_text(family=font_name, color='black', size=12),
               plot.title=element_text(family=font_name, size=14),
               axis.ticks.length=unit(0.5, 'lines'),
               plot.margin=unit(c(1, 1, 1, 1), 'lines'))
plot(g)
dev.off()
