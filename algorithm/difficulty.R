library(extrafont)
library(jsonlite)
library(tidyverse)

g_font_name <- "Migu 1M"
rating_chars <- c("灰", "茶", "緑", "水", "青", "黄", "橙", "赤")
rating_colors <- c(
  "#808080", "#804000", "#008000", "#00C0C0",
  "#0000FF", "#C0C000", "#FF8000", "#FF0000"
)
g_incoming_data_dir <- "incoming_data"

## 問題を読み込む
read_task_difficulties <- function(path) {
  contests <- jsonlite::read_json(path)

  purrr::reduce(.x = seq_len(NROW(contests)), .init = NULL, .f = function(acc, i) {
    tasks <- contests[i]
    name <- names(tasks)
    df <- as_tibble(do.call(rbind.data.frame, tasks)) %>%
      dplyr::mutate(contest = name, .before = 1)
    if (is.null(acc)) {
      df
    } else {
      dplyr::bind_rows(acc, df)
    }
  })
}

## コンテストを限定する
select_contests <- function(df_contest, contest_name, out_dirname) {
  inner_name <- stringr::str_to_lower(contest_name)
  pattern <- paste0("^(", inner_name, ")(.*)")

  df_target <- df_contest %>%
    tidyr::extract(col = contest, into = c("contest", "task"), regex = "(.*)_([^_]+$)") %>%
    dplyr::filter(stringr::str_starts(stringr::str_to_lower(contest), inner_name)) %>%
    tidyr::extract(col = contest, into = c("name", "id"), regex = pattern, remove = FALSE) %>%
    dplyr::mutate(name = stringr::str_to_lower(name))

  tasks <- c("a", "b", "c", "d", "e", "f", "g", "h")
  df_tasks <- df_target %>%
    dplyr::select(all_of(c("contest", "task", "difficulty"))) %>%
    na.omit() %>%
    dplyr::filter(task %in% tasks) %>%
    dplyr::arrange(contest, task) %>%
    dplyr::mutate(contest = stringr::str_to_upper(contest)) %>%
    dplyr::mutate(rating = 1 + pmax(0, floor(difficulty / 400))) %>%
    dplyr::mutate(mark = ifelse(rating < 10, paste(rating), NA)) %>%
    dplyr::select(-all_of(c("difficulty", "rating"))) %>%
    tidyr::pivot_wider(id_cols = "contest", names_from = "task", values_from = "mark") %>%
    dplyr::mutate(across(everything(), ~ replace_na(.x, " "))) %>%
    dplyr::mutate(sep = ":", .after = contest) %>%
    tidyr::unite("tasks", everything(), sep = "", remove = FALSE, na.rm = FALSE) %>%
    dplyr::mutate(tasks = stringr::str_trim(tasks, side = "right"))

  if (!is.na(out_dirname)) {
    out_filename <- file.path(out_dirname, paste0("difficulty_auto_", contest_name, ".csv")) %>%
      stringr::str_to_lower()
    df_tasks %>%
      dplyr::select(tasks) %>%
      readr::write_csv(file = out_filename, col_names = FALSE)
  }

  df_target
}

## 結果を読み込む
read_results <- function(path, contest_name) {
  name <- stringr::str_to_lower(contest_name)
  cin <- file(path, "r")
  lines <- readLines(cin)
  close(cin)

  purrr::reduce(.x = lines, .init = NULL, .f = function(acc_all, line) {
    # 解けたかどうかを記したテキストファイルを読む
    # 1行1コンテストに対応する。コンテスト名(もしあれば)三桁のID, ':', A..H問題の結果を一行に書く。例えば
    # ABC277:  ++-
    # はコンテストABC277のA,Bは解いていない、C,Dは解けた、Eは解けなかった、F以降は解いていない
    # ことを示す。+は解けた、空白は解いていない、それ以外は解けなかったことを示す
    # :の直後にA問題の結果を置く。:と結果の間には何も書かない。
    matched <- stringr::str_match(line, "^[\\D]*(\\d{3}):(.{1,8})")
    if (is.na(matched[1, 1])) {
      acc_all
    } else {
      id <- matched[1, 2]
      results <- strsplit(matched[1, 3], "")[[1]]
      df_contest <- purrr::reduce(.x = seq_len(NROW(results)), .init = NULL, .f = function(acc_contest, i) {
        result <- results[i]
        task <- c("a", "b", "c", "d", "e", "f", "g", "h")[i]
        score <- NA
        if (result == "+") {
          score <- 1
        } else if (result != " ") {
          score <- -1
        }
        df <- tibble::tibble(name = name, id = id, task = task, score = score)
        if (is.null(acc_contest)) {
          df
        } else {
          dplyr::bind_rows(acc_contest, df)
        }
      })

      if (is.null(acc_all)) {
        df_contest
      } else {
        dplyr::bind_rows(acc_all, df_contest)
      }
    }
  })
}

## 問題と結果を統合する
merge_score <- function(df_tasks, df_results) {
  df_left <- df_tasks %>%
    dplyr::mutate(id = as.numeric(id))

  df_right <- df_results %>%
    dplyr::mutate(id = as.numeric(id))

  dplyr::inner_join(df_left, df_right) %>%
    dplyr::filter(!is.na(score)) %>%
    dplyr::filter(abs(score) == 1) %>%
    dplyr::mutate(difficulty = pmax(0, difficulty)) %>%
    dplyr::arrange(name, id, task) %>%
    dplyr::mutate(rank = 1 + pmax(0, difficulty %/% 400), .after = difficulty)
}

## 解けたかどうかを散布図にする
draw_scatter_plot <- function(df_score, contest_name) {
  id_minmax <- range(df_score$id)
  id_center <- (id_minmax[2] + id_minmax[1]) / 2.0
  id_tick <- 1.4 / (id_minmax[2] - id_minmax[1])
  df_drawn <- df_score %>%
    dplyr::mutate(x = difficulty, y = score + (id - id_center) * id_tick)

  rating_range <- 400
  rating_min <- rating_range * (seq_len(NROW(rating_colors)) - 1)
  rating_max <- rating_range * seq_len(NROW(rating_colors))
  df_grade <- tibble(start = rating_min, end = rating_max, color = factor(seq_len(NROW(rating_colors))))

  title <- paste0(contest_name, "を時間無制限で解けたかどうか")
  g <- ggplot()
  g <- g + geom_rect(data = df_grade, aes(xmin = start, xmax = end, ymin = -Inf, ymax = Inf, fill = color), fill = rating_colors)
  g <- g + geom_point(data = df_drawn, aes(x = x, y = y), shape = 17, size = 3, color = "gray80")
  g <- g + scale_y_continuous(breaks = c(-1, 1), labels = c("解けなかった", "解けた"))
  g <- g + geom_hline(yintercept = 0)
  g <- g + labs(title = title, x = "Difficulty", y = "解けたかどうか")
  g <- g + theme_bw()
  g <- g + theme(
    text = element_text(family = g_font_name),
    axis.text = element_text(family = g_font_name, size = 14),
    axis.title.x = element_text(family = g_font_name, size = 16),
    axis.title.y = element_blank(),
    strip.text = element_text(family = g_font_name, size = 18),
    plot.title = element_text(family = g_font_name, size = 18)
  )
  g
}

## Ratingごとの正解率をヒストグラムにする
draw_histogram <- function(df_score, contest_name) {
  labels <- c("解けた", "解けなかった")
  df_drawn <- df_score %>%
    dplyr::mutate(color = factor(labels[(3 - score) / 2])) %>%
    dplyr::mutate(color = forcats::fct_relevel(color, labels)) %>%
    dplyr::select(c("rank", "color"))

  title <- paste0(contest_name, "を時間無制限で解けたかどうか")
  g <- ggplot()
  g <- g + geom_bar(data = df_drawn, aes(x = rank, fill = color), color = "black")
  g <- g + scale_fill_manual(values = c("royalblue", "gray80"))
  g <- g + scale_x_continuous(breaks = seq_len(NROW(rating_chars)), labels = rating_chars)
  g <- g + labs(title = title, x = "Rating", y = "問題数")
  g <- g + theme_bw()
  g <- g + theme(
    text = element_text(family = g_font_name),
    axis.text = element_text(family = g_font_name, size = 14),
    axis.title = element_text(family = g_font_name, size = 16),
    strip.text = element_text(family = g_font_name, size = 18),
    plot.title = element_text(family = g_font_name, size = 18),
    legend.position = "bottom",
    legend.title = element_blank(),
    legend.text = element_text(size = 16)
  )
  g
}

## コンテストを限定して結果を集計する
execute_all <- function(result_filename, df_contests, contest_name, out_dirname) {
  df_target <- select_contests(
    df_contest = df_contests, contest_name = contest_name,
    out_dirname = out_dirname
  )
  df_results <- read_results(result_filename, contest_name = contest_name)
  df_score <- merge_score(df_tasks = df_target, df_results = df_results)

  g_scatter <- draw_scatter_plot(df_score = df_score, contest_name = contest_name)
  ggsave("images/score.png", plot = g_scatter, width = 6, height = 4)
  g_histogram <- draw_histogram(df_score = df_score, contest_name = contest_name)
  ggsave("images/hist.png", plot = g_histogram, width = 6, height = 4)

  list(
    df_target = df_target, df_results = df_results, df_score = df_score,
    g_scatter = g_scatter, g_histogram = g_histogram
  )
}

# 難易度をAtCoder Problemsからダウンロードする
# https://github.com/kenkoooo/AtCoderProblems/blob/master/doc/api.md
# からリンクされている
# https://kenkoooo.com/atcoder/resources/problem-models.json
# 何度もダウンロードしないように、解析はローカルのファイルを読む
df_all_contests <- read_task_difficulties(file.path(g_incoming_data_dir, "problem-models.json"))

# ABC (AtCoder Beginner Contest) に限る
result <- execute_all(
  result_filename = "results.txt",
  df_contests = df_all_contests, contest_name = "ABC",
  out_dirname = g_incoming_data_dir
)
