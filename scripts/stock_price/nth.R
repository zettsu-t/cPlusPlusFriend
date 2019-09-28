library(dplyr)
library(forcats)
library(kernlab)

xs <- 0:99
suffixes <- as.factor(sapply(xs, function(x) {
    suffix <- if ((x %/% 10) == 1) {
        "th"
    } else {
        c("st", "nd", "rd", "th")[min(3, (x + 9) %% 10) + 1]
    }
}))

n_train <- 75

digit10 <- fct_inorder(as.factor(xs %/% 10))
digit1 <- fct_inorder(as.factor(xs %% 10))
df <- data.frame(digit10=digit10, digit1=digit1, suffix=suffixes)

train_xs <- sample(xs, n_train, replace = FALSE)
test_xs <- setdiff(xs, train_xs)
df_train <- df[train_xs,]
df_test <- df[test_xs,]

fit <- ksvm(suffix ~ ., data=df_train)
df_test$predicted <- predict(fit, df_test %>% select(-c(suffix)))
suffix_diff <- df_test$suffix == df_test$predicted

print(fit)
print(suffix_diff)
