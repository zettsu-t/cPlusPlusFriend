library(tidyverse)
library(extrafont)
library(pals)
library(purrrlyr)
library(zipangu)

## データの出典
## NHK放送文化研究所「国民生活時間調査」
## 全員平均時間量、男女年層別、 1995, 2000, 2005, 2010, 2015, 2020年調査
## https://www.nhk.or.jp/bunken/yoron-jikan/
## 以下の記載に基づいて、コンテンツを利用しています
## https://www.nhk.or.jp/bunken/yoron-jikan/terms-of-service.html
##
## ダウンロードした *_4shihyo_danjonenso.csv を、
## このスクリプトの incoming/ ディレクトリに置いてください

read_data_all <- function(in_dirname) {
  filenames <- sort(Sys.glob(file.path(in_dirname, "*shihyo_danjonenso.csv")))

  purrr::reduce(.x = filenames, .init = NULL, .f = function(acc, filename) {
    year <- as.numeric(stringr::str_extract(filename, "\\d{4}"))
    if (is.na(year)) {
      acc
    } else {
      df <- readr::read_csv(filename)
      df$調査年 <- year
      if (is.null(acc)) {
        df
      } else {
        dplyr::bind_rows(acc, df)
      }
    }
  }) %>%
    dplyr::mutate(`層` = zipangu::str_conv_zenhan(`層`, to = "hankaku")) %>%
    dplyr::mutate(`性別` = stringr::str_extract(`層`, "(男|女)")) %>%
    dplyr::mutate(`年齢` = stringr::str_extract(`層`, "\\d+.(以上)?")) %>%
    dplyr::mutate(`年齢階層` = as.numeric(stringr::str_extract(`年齢`, "\\d+"))) %>%
    dplyr::mutate(`生年` = (`調査年` - `年齢階層`)) %>%
    purrrlyr::by_row(function(input) {
      gender <- input$`性別`
      year_end <- input$`生年`
      year_start <- year_end - 9
      prefix <- input$`性別`
      if (stringr::str_detect(input$`年齢`, "以上")) {
        sprintf("%s -%s生", prefix, year_end)
      } else {
        sprintf("%s %s-%s生", prefix, year_start, year_end)
      }
    }, .collate = c("cols"), .to = "cohort") %>%
    dplyr::mutate(len_cohort = stringr::str_length(cohort)) %>%
    dplyr::arrange(desc(`性別`), `生年`, len_cohort) %>%
    dplyr::mutate(cohort = factor(cohort)) %>%
    dplyr::mutate(cohort = forcats::fct_inorder(cohort)) %>%
    dplyr::mutate(`層` = factor(`層`)) %>%
    dplyr::mutate(`層` = forcats::fct_inorder(`層`))
}

filter_by_action <- function(df, action, days_in_week) {
  df %>%
    dplyr::filter((`行動` == action) & (`曜日` == days_in_week)) %>%
    dplyr::rename(value = `行為者(％)`, year = `調査年`, group = `層`, birth = `生年`, gender = `性別`) %>%
    dplyr::select(c("value", "year", "group", "cohort", "birth", "gender")) %>%
    dplyr::mutate(group = factor(group)) %>%
    dplyr::mutate(group = forcats::fct_inorder(group))
}

draw_by_survey_years <- function(df, action, days_in_week, key, color_func, webapp, use_plotly) {
  title_prefix <- "国民生活時間調査 全員平均時間量、男女年層別 "
  title_filter <- sprintf("%s(%s)\n", action, days_in_week)
  title_source1 <- "出典: NHK放送文化研究所「国民生活時間調査」"
  title_lf1 <- "\n"
  title_source2 <- "(https://www.nhk.or.jp/bunken/yoron-jikan/)"

  if (webapp) {
    if (use_plotly) {
      title_lf1 <- ""
      title_source2 <- ""
    }
  }
  title <- paste0(
    title_prefix, title_filter, title_source1, title_lf1,
    title_source2, " を図の作者が編集・加工して作成"
  )

  legend_title <- if (key == "cohort") {
    "性別と生年(調査年から年齢を引いたもの)\n年齢は10..60代と70歳以上"
  } else {
    "性別と年代"
  }

  n_key <- NROW(unique(df[[key]])) %/% 2
  n_gender <- NROW(unique(df$gender))
  color_set <- rep(color_func(n_key), n_gender)
  linetype_set <- rep(c("solid", "dashed"), each = n_key)
  shape_set <- rep(c(2, 4), each = n_key)
  font_name <- "Migu 1M"

  point_stroke <- 2
  legend_text <- 12
  axis_text <- 14
  axis_title <- 16
  plot_title <- 18
  if (webapp) {
    point_stroke <- 0.5
    legend_text <- 12
    axis_text <- 14
    axis_title <- 16
    plot_title <- 12
  }

  jitter <- position_jitter(width = 0.1, height = 0)
  g <- ggplot(df, aes_string(x = "year", y = "value", color = key, linetype = key, shape = key))
  g <- g + geom_line(size = 1)
  g <- g + geom_point(position = jitter, size = 4, stroke = point_stroke)
  g <- g + scale_color_manual(values = color_set)
  g <- g + scale_linetype_manual(values = linetype_set)
  g <- g + scale_shape_manual(values = shape_set)
  if (!webapp) {
    g <- g + scale_y_continuous(limits = c(0, 100))
  }
  g <- g + ggtitle(title)
  g <- g + xlab("調査年")
  g <- g + ylab("行為者[%]")
  g <- g + guides(
    color = guide_legend(
      title = legend_title, ncol = 2, override.aes =
        list(color = color_set, linetype = linetype_set, shape = shape_set)
    ),
    linetype = guide_legend(title = legend_title, ncol = 2),
    shape = guide_legend(title = legend_title, ncol = 2)
  )
  g <- g + theme_bw()
  g <- g + theme(
    legend.position = "right",
    legend.key.width = unit(3.5, "line"),
    legend.text = element_text(family = font_name, color = "black", size = legend_text),
    text = element_text(family = font_name),
    axis.text.x = element_text(family = font_name, color = "black", size = axis_text),
    axis.text.y = element_text(family = font_name, color = "black", size = axis_text),
    axis.title.x = element_text(family = font_name, color = "black", size = axis_title),
    axis.title.y = element_text(family = font_name, color = "black", size = axis_title),
    plot.title = element_text(family = font_name, size = plot_title),
    axis.ticks.length = unit(0.5, "lines")
  )
  g
}

draw_by_survey_years_for_webapp <- function(df, action, days_in_week, key, color_func, use_plotly) {
  draw_by_survey_years(df, action, days_in_week, key, color_func, webapp = TRUE, use_plotly = use_plotly)
}

draw_by_survey_years_for_png <- function(df, action, days_in_week, key, color_func) {
  draw_by_survey_years(df, action, days_in_week, key, color_func, webapp = FALSE, use_plotly = FALSE)
}

draw_by_cohort_ages <- function(df, action, days_in_week, out_dirname, out_filename) {
  g <- draw_by_survey_years_for_png(
    df = df, action = action, days_in_week = days_in_week,
    key = "cohort", color_func = pals::stepped
  )

  dir.create(path = out_dirname, showWarnings = TRUE, recursive = TRUE)
  filename <- file.path(out_dirname, paste0(out_filename, "_cohort.png"))
  png(filename = filename, width = 900, height = 600)
  plot(g)
  dev.off()

  g <- draw_by_survey_years_for_png(
    df = df, action = action, days_in_week = days_in_week,
    key = "group", color_func = pals::tol
  )
  filename <- file.path(out_dirname, paste0(out_filename, "_age.png"))
  png(filename = filename, width = 900, height = 600)
  plot(g)
  dev.off()
}

execute_all <- function(in_dirname, out_dirname, actions) {
  df_survey <- read_data_all(in_dirname = in_dirname)
  days_in_week_set <- unique(df_survey$`曜日`)

  purrr::map(actions, function(action) {
    purrr::map(days_in_week_set, function(days_in_week) {
      df_sub <- filter_by_action(df = df_survey, action = action, days_in_week = days_in_week)
      out_filename <- paste0(action, "_", days_in_week) %>%
        stringr::str_replace("[・（）()]", "_")
      draw_by_cohort_ages(
        df = df_sub, action = action, days_in_week = days_in_week,
        out_dirname = out_dirname, out_filename = out_filename
      )
    })
    0
  })
}
