## Shinyで日本語の列名が読めないときの回避策
options(encoding = "UTF-8")
source("living_time_survey.R")

## 読み込んだデータ
g_df_survey <- read_data_all(in_dirname = "incoming")

## ui.Rから選択肢が見えるようにする
g_action_set <- unique(g_df_survey$`行動`)
g_days_in_week_set <- unique(g_df_survey$`曜日`)
