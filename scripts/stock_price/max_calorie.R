## Data source : https://github.com/marushosummers/Saizeriya_1000yen
## SQLite : https://qiita.com/toshi71/items/6802e64098b4012a8e5a
## Other implementations
## https://qiita.com/tanakh/items/a1fb13f78e0576415de3
## https://qiita.com/YSRKEN/items/dfc8604eb8598e5e9076

library(RSQLite)
library(lpSolve)

filename <- "saizeriya.db"
conn <- dbConnect(RSQLite::SQLite(), filename)
menu_df <- dbGetQuery(conn, 'SELECT name, price, calorie FROM menu')
menu_df$price <- as.integer(menu_df$price)
menu_df$calorie <- as.integer(menu_df$calorie)
dbDisconnect(conn)

find_combination <- function(df, total) {
    n <- NROW(df$price)
    ## scale=0 and all.int (not all.bin) are required to find the known optimal
    s <- lp("max", df$calorie, rbind(diag(n), t(df$price), diag(n)),
            c(rep("<=", n+1), rep(">=", n)),
            c(rep(1, n), total, rep(0, n)),
            scale=0, all.int=TRUE)
    print(s$solution)
    which(s$solution == 1)
}

print_combination <- function(df, total) {
    result <- find_combination(df, total)
    df_selected <- df[result,]
    print(df_selected)
    print(sum(df_selected$calorie))
    print(sum(df_selected$price))
    result
}

result_1000 <- print_combination(menu_df, 1000)
result_10000 <- print_combination(menu_df, 10000)
