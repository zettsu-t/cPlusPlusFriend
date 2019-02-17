library(ggplot2)
library(ROCR)
library(data.table)
library(plyr)
library(dplyr)

## Data source
## https://dn-sundai.benesse.ne.jp/dn/center/doukou/daigaku/1140/index.html
## Download a spreadsheet from above, extract columns of scores,
## numbers of passed or failed applicants, and save them as a score.csv.

df <- fread('incoming/score.csv')
data <- rbind(data.table(score=rep(df$score, df$pass), passed=rep(rep(1,NROW(df)), times=df$pass)),
              data.table(score=rep(df$score, df$fail), passed=rep(rep(0,NROW(df)), times=df$fail))) %>%
    arrange(desc(score), desc(passed))

## Cited from http://www.nemotos.net/?p=836
pred <- prediction(data[['score']], data[['passed']])

perf <- performance(pred, "tpr", "fpr")
plot(perf)

perf <- performance(pred, "sens", "spec")
plot(perf)
