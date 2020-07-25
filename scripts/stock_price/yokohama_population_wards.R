library(assertthat)
library(directlabels)
library(dplyr)
library(extrafont)
library(forcats)
library(ggplot2)
library(purrr)
library(readxl)
library(stringr)

## Data source
## https://www.city.yokohama.lg.jp/city-info/yokohamashi/tokei-chosa/portal/jinko/choki.html
read_population_sheet <- function(in_filename) {
    sheet_names <- readxl::excel_sheets(in_filename)
    purrr::map(sheet_names, function(sheet_name) {
        df <- suppressMessages(readxl::read_excel(path=in_filename, sheet=sheet_name, col_names=FALSE, skip=4)) %>%
            dplyr::select(c(2, 3, 5))
        colnames(df) <- c('year', 'area', 'population')
        df <- df %>%
            dplyr::mutate(year=as.integer(year)) %>%
            dplyr::filter(!is.na(year)) %>%
            dplyr::filter((year >= 1950) & (year < 2020))
        df$ward <- sheet_name
        df
    }) %>% dplyr::bind_rows()
}

merge_wards <- function(df, from_ward, to_ward) {
    df_from <- df %>% dplyr::filter(ward==from_ward)
    df_to <- df %>% dplyr::filter(ward %in% setdiff(to_ward, from_ward))

    df_merged <- dplyr::bind_rows(df_from, df_to) %>%
        dplyr::select(-c('ward')) %>%
            dplyr::group_by(year) %>%
            dplyr::summarize_all(list(sum)) %>%
            dplyr::ungroup()

    df_merged$ward <- paste0('旧', from_ward)
    dplyr::bind_rows(df, df_merged)
}

parse_wards <- function(in_filename, wards, old_wards, ward_map) {
    df <- read_population_sheet(in_filename)
    ward_names <- unique(df$ward)
    old_ward_names <- stringr::str_replace(old_wards, '旧', '')
    assertthat::assert_that(NROW(setdiff(ward_names, wards)) == 0)
    assertthat::assert_that(NROW(setdiff(wards, ward_names)) == 0)
    assertthat::assert_that(NROW(setdiff(old_ward_names, ward_names)) == 0)
    assertthat::assert_that(NROW(setdiff(old_ward_names, purrr::map_chr(ward_map, ~ .x$from_ward))) == 0)

    ## https://www.city.yokohama.lg.jp/city-info/yokohamashi/ku-shokai/division.html
    df <- purrr::reduce(.x=ward_map, .init=df, .f=function(acc, ward_set) {
        from_ward <- ward_set$from_ward
        to_ward <- ward_set$to_wards
        assertthat::assert_that(NROW(setdiff(unique(df$ward), c(wards, old_wards))) == 0)
        assertthat::assert_that(NROW(setdiff(unique(from_ward), wards)) == 0)
        assertthat::assert_that(NROW(setdiff(unique(to_ward), wards)) == 0)
        merge_wards(df=acc, from_ward=from_ward, to_ward=to_ward)
    })
    assertthat::assert_that(NROW(setdiff(unique(df$ward), c(wards, old_wards))) == 0)

    df %>%
        dplyr::mutate(ward=factor(ward)) %>%
        dplyr::mutate(ward=forcats::fct_relevel(ward, c(wards, old_wards)))
}

draw_wards <- function(df, wards, old_wards, ward_colors, ward_styles) {
    font_name <- 'Migu 1M'
    font_size <- 16
    png_filename <- 'wards.png'
    png(png_filename, width = 1280, height = 640)

    g <- ggplot(df, aes(x=year, y=population, color=ward, linetype=ward, label=ward))
    g <- g + geom_line(alpha=0.75, size=1)
    g <- g + geom_dl(method=list(dl.combine('first.points', 'last.points'), cex=0.75, fontfamily=font_name))
    g <- g + scale_x_continuous(breaks=seq(1950, 2020, 10), limits=c(1945, 2025))
    g <- g + scale_y_continuous(breaks=seq(0, 1100000, 100000), limits=c(0, 1100000))
    g <- g + scale_color_manual(values=ward_colors)
    g <- g + scale_linetype_manual(values=ward_styles)
    g <- g + guides(color=guide_legend(ncol=1))
    g <- g + theme_bw()
    g <- g + theme(text=element_text(family=font_name),
                   legend.key.width=unit(5, "line"),
                   legend.position='right',
                   legend.text=element_text(size=font_size),
                   legend.title=element_blank(),
                   axis.text=element_text(family=font_name, size=font_size),
                   axis.title=element_text(family=font_name, size=font_size),
                   strip.text=element_text(family=font_name, size=font_size),
                   plot.title=element_text(family=font_name, size=font_size))
    plot(g)
    dev.off()
}

## North to south
wards <- c('鶴見',
           '港北', '緑', '青葉', '都筑',
           '神奈川', '西', '中',
           '保土ケ谷', '旭',
           '南', '港南',
           '戸塚', '瀬谷', '栄', '泉',
           '磯子', '金沢')

old_wards <- c('旧港北', '旧保土ケ谷', '旧南', '旧戸塚')

ward_colors <- c('black',
                 'navy', 'dodgerblue4', 'dodgerblue2', 'skyblue2',
                 'chartreuse4', 'chartreuse3', 'chartreuse1',
                 'orchid4', 'orchid2',
                 'slategray4', 'slategray2',
                 'orangered', 'orange4', 'orange3', 'orange1',
                 'yellow4', 'yellow3',
                 'navy', 'orchid4', 'slategray4', 'orangered', 'yellow4')

ward_map <- list(list(from_ward='港北', to_wards=c('緑', '青葉', '都筑')),
                 list(from_ward='保土ケ谷', to_wards=c('旭')),
                 list(from_ward='南', to_wards=c('港南')),
                 list(from_ward='戸塚', to_wards=c('瀬谷', '栄', '泉')))

ward_styles <- c(rep('solid', NROW(wards)), rep('dashed', NROW(old_wards)))

in_filename <- 'incoming/2.xlsx'
df <- parse_wards(in_filename=in_filename, wards=wards, old_wards=old_wards, ward_map=ward_map)
draw_wards(df=df, wards=wards, old_wards=old_wards, ward_colors=ward_colors, ward_styles=ward_styles)
