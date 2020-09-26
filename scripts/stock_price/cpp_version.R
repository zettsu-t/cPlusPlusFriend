## Windowsのバージョンとリリース年、の何番煎じか
library(tidyverse)
library(dplyr)
library(forcats)
library(ggplot2)
library(extrafont)

ver_set <- c(98, 03, 11, 14, 17, 20, 23)
label_set <- c('C++98', 'C++03', 'C++11', 'C++14', 'C++17', 'C++20', 'C++2b')
label_set <- factor(label_set) %>%
    forcats::fct_relevel(label_set)
year_set <- c(1998, 2003, 2011, 2014, 2017, 2020, 2023)
color_set <- c('goldenrod3', 'goldenrod4', 'dodgerblue2', 'dodgerblue3', 'dodgerblue4', 'steelblue2', 'steelblue3')

df <- tibble(ver=ver_set, year=year_set, label=label_set)

font_name <- 'Migu 1M'
font_size <- 16

png(filename='cpp_ver.png', width=640, height=400)
g <- ggplot(df, aes(x=year, y=ver, color=label, fill=label))
g <- g + geom_bar(stat='identity', width=1)
g <- g + geom_text(aes(label=label), vjust=-0.2, color='black', size=5, family=font_name)
g <- g + scale_color_manual(values=color_set)
g <- g + scale_fill_manual(values=color_set)
g <- g + xlab('Year')
g <- g + ylab('')
g <- g + scale_x_continuous(breaks=year_set)
g <- g + theme(text=element_text(family=font_name),
               legend.position='none',
               legend.text=element_text(size=font_size),
               legend.title=element_blank(),
               axis.text=element_text(family=font_name, size=font_size),
               axis.title=element_text(family=font_name, size=font_size),
               strip.text=element_text(family=font_name, size=font_size),
               plot.title=element_text(family=font_name, size=font_size))
plot(g)
dev.off()
