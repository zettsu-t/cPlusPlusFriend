## Data source
## https://www.e-stat.go.jp/gis

library(tidyverse)
library(dplyr)
library(forcats)
library(ggplot2)
library(purrr)
library(readr)
library(tibble)
library(extrafont)
library(sf)

df <- readr::read_csv('economics/tblT000918H5238_5339.csv') %>%
    dplyr::select('KEY_CODE', 'T000918014') %>%
    dplyr::mutate(KEY_CODE=as.factor(KEY_CODE)) %>%
    dplyr::rename(n=T000918014) %>%
    dplyr::filter(n>0)

shape_filenames <- c('economics/MESH05238.shp', 'economics/MESH05239.shp',
                     'economics/MESH05338.shp', 'economics/MESH05339.shp')
mesh <- purrr::reduce(.x=shape_filenames, .init=NULL, .f=function(acc, x) {
    dplyr::bind_rows(acc, sf::st_read(x))
})

pref_shape <- sf::st_read('economics/h28ca14.shp') %>%
    dplyr::arrange('CITY_NAME') %>%
    dplyr::filter(!is.na(CITY_NAME))

## Factorize cities in an explicit order
city_names <- unique(as.character(na.omit(pref_shape$CITY_NAME)))
pref_shape$CITY_NAME <- as.factor(pref_shape$CITY_NAME)
pref_shape$CITY_NAME <- forcats::fct_relevel(pref_shape$CITY_NAME, levels=city_names)

## group_by does not work
city_shape <- pref_shape %>%
    dplyr::select('CITY_NAME', 'geometry') %>%
    split(.$CITY_NAME) %>%
    lapply(sf::st_union) %>%
    do.call(c, .) %>%
    sf::st_cast()

pref <- tibble(city=city_names, geometry=city_shape)
geo_map <- sf::st_intersection(mesh, pref$geometry)
geo_data <- dplyr::inner_join(geo_map, df, by=c('KEY_CODE'))

font_name <- 'Migu 1M'
font_size <- 14
label_font_size <- 2
legend_font_size <- 12

g <- ggplot()
g <- g + geom_sf(data=pref, mapping=aes(geometry=geometry))
g <- g + geom_sf_text(data=pref, mapping=aes(geometry=geometry, label=city),
                      size=label_font_size, family=font_name, color='black')
g <- g + geom_sf(mapping=aes(geometry=geometry, fill=n), data=geo_data, color=NA, alpha=0.6, lwd=0)
g <- g + ggtitle('宿泊業，飲食サービス業 (神奈川県 : 500mメッシュ)')
g <- g + scale_fill_gradient2(low='white', high='purple4', name=guide_legend(title='事業所数'),
                              breaks=c(1,3,10,31,100,310), guide='colourbar', trans='log10')
g <- g + theme(text=element_text(family=font_name),
               legend.position='right',
               legend.text=element_text(size=legend_font_size),
               legend.title=element_text(family=font_name, size=legend_font_size),
               axis.title=element_blank(),
               axis.text=element_blank(),
               axis.ticks=element_blank(),
               strip.text=element_text(family=font_name, size=font_size),
               plot.title=element_text(family=font_name, size=font_size))
plot(g)
ggsave('outgoing/kanagawa_hotel_restaurant.png', plot=g, dpi=200)
ggsave('outgoing/kanagawa_hotel_restaurant.svg', plot=g)
