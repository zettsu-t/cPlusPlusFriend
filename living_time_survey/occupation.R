library(tidyverse)
library(extrafont)
library(ggpubr)
library(pals)

## データの出典
## NHK放送文化研究所「国民生活時間調査」
## 全員平均時間量、職業別、 1995, 2000, 2005, 2010, 2015, 2020年調査
## https://www.nhk.or.jp/bunken/yoron-jikan/
## 以下の記載に基づいて、コンテンツを利用しています
## https://www.nhk.or.jp/bunken/yoron-jikan/terms-of-service.html

read_data_all <- function(in_dirname, excluded_occupation_set) {
  filenames <- sort(Sys.glob(file.path(in_dirname, "*_4shihyo_shokugyo.csv")))

  df <- purrr::reduce(.x = filenames, .init = NULL, .f = function(acc, filename) {
    year <- as.numeric(stringr::str_extract(filename, "\\d{4}"))
    if (is.na(year)) {
      acc
    } else {
      df <- readr::read_csv(filename)
      df$year <- year
      if (is.null(acc)) {
        df
      } else {
        dplyr::bind_rows(acc, df)
      }
    }
  }) %>%
      dplyr::filter((`曜日` == "平日") & (`行動` == "仕事")) %>%
      dplyr::rename(occupation = `層`, ratio = `行為者(％)`,
                    participant_time = `行為者平均時間量(時間:分)`, all_time = `全員平均時間量(時間:分)`) %>%
      dplyr::select(c("year", "occupation", "participant_time", "all_time", "ratio")) %>%
      dplyr::filter(!(occupation %in% excluded_occupation_set))

  occupations <- (df %>%
    dplyr::filter(year == 2015) %>%
    dplyr::arrange(desc(participant_time)))$occupation

  df %>%
    dplyr::mutate(occupation = factor(occupation)) %>%
    dplyr::mutate(occupation = forcats::fct_relevel(occupation, occupations))
}

draw_plots_in_one <- function(df) {
  font_name <- "Migu 1M"

  set_font_size <- function(g) {
    g + theme(
      legend.position = "right",
      legend.key.width = unit(3, "line"),
      legend.text = element_text(family = font_name, color = "black", size = 14),
      text = element_text(family = font_name),
      axis.text.x = element_text(family = font_name, color = "black", size = 14),
      axis.text.y = element_text(family = font_name, color = "black", size = 14),
      axis.title.x = element_text(family = font_name, color = "black", size = 16),
      axis.title.y = element_text(family = font_name, color = "black", size = 16)
    )
  }

  n_occupations <- NROW(unique(df$occupation))
  color_set <- pals::tol(n_occupations)
  title <- "国民生活時間調査 全員平均時間量、職業別 (平日の仕事 : 学生、主婦、 家庭婦人、無職を除く)\n出典: NHK放送文化研究所「国民生活時間調査」(https://www.nhk.or.jp/bunken/yoron-jikan/) を図の作者が編集・加工して作成。"

  g1 <- ggplot(df, aes(x = year, color = occupation))
  g1 <- g1 + geom_line(aes(y = participant_time), size = 1.5, linetype = "solid")
  g1 <- g1 + geom_line(aes(y = all_time), size = 1.5, linetype = "dashed")
  g1 <- g1 + geom_point(aes(y = all_time), size = 4, shape = 16)
  g1 <- g1 + scale_color_manual(values = color_set)
  g1 <- g1 + xlab("調査年")
  g1 <- g1 + ylab("行為者平均時間量(実線)\n全員平均時間量(破線)[時:分:秒]")
  g1 <- g1 + guides(color = guide_legend(title = "職業"))
  g1 <- g1 + guides(color = guide_legend(title = "職業", ncol = 1, override.aes = list(color = color_set)))
  g1 <- set_font_size(g1)

  g2 <- ggplot(df, aes(x = year, color = occupation))
  g2 <- g2 + geom_line(aes(y = ratio), size = 1.5, linetype = "solid")
  g2 <- g2 + geom_point(aes(y = ratio), size = 4, shape = 15)
  g2 <- g2 + scale_color_manual(values = color_set)
  g2 <- g2 + xlab("調査年")
  g2 <- g2 + ylab("行為者[％]")
  g2 <- g2 + guides(color = guide_legend(title = "職業"))
  g2 <- g2 + guides(color = guide_legend(title = "職業", ncol = 1, override.aes = list(color = color_set)))
  g2 <- set_font_size(g2)

  figure <- ggarrange(g1, g2, ncol = 1)
  annotate_figure(figure, top = text_grob(title, family = font_name, size = 14))
}

df <- read_data_all(in_dirname = "incoming", excluded_occupation_set = c("学生", "主婦", "家庭婦人", "無職"))
out_dirname <- "outgoing"
dir.create(path = out_dirname, showWarnings = TRUE, recursive = TRUE)

png(filename = file.path(out_dirname, "occupation.png"), width = 1000, height = 750)
plot(draw_plots_in_one(df))
dev.off()
