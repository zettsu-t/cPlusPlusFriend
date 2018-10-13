library(ggplot2)
library(directlabels)

df <- read.csv('incoming/yokohama.csv')
df$date <- as.Date(df$date)
df$diff_pop_ratio <- 100.0 * df$diff_population / (df$population - df$diff_population)

df.sum <- df[df['region'] == 'A',]
df.north <- df[df['region'] == 'N', ]
df.south <- df[df['region'] == 'S',]

draw_chart <- function(df_arg) {
    g <- ggplot(df_arg, aes(x=date, y=diff_pop_ratio, color=ward_jp))
    g <- g + geom_line(size=1)
    g <- g + scale_x_date(date_breaks='2 months')
    g <- g + theme(axis.text.x=element_text(angle=-90, vjust=0.5), legend.title=element_blank())
    g <- g + xlab('Date')
    g <- g + ylab('Diff(population) [%]')
    g <- g + geom_dl(aes(label=ward_jp), method=list(dl.combine('first.points', 'last.points'), color='black', cex=0.7))
    g <- g + geom_line(data=df.sum, aes(x=date, y=diff_pop_ratio), color='black', size=1.2)
    g <- g + annotate('text',x=df$date[5], y=0, label='All wards')
    plot(g)
}

draw_chart(df.north)
draw_chart(df.south)
