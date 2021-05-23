options(encoding = "UTF-8")
source("living_time_survey.R")
execute_all(in_dirname = "incoming", out_dirname = "outgoing", actions = c("新聞", "テレビ"))
