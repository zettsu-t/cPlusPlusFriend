## Find a nearest value in a vector
library(data.table)
library(DescTools)
library(ggplot2)
library(microbenchmark)

data_step <- 0.01
data_delta <- data_step / 2.0

conv <- function(x) {
    (x + data_delta) * (x + data_delta)
}

n_samples_set <- c(100, 1000, 10000, 40000)
all_ys <- conv(seq(1, max(n_samples_set), data_step))

make_subdt <- function(index) {
    n <- n_samples_set[index]
    dt <- data.table(value=all_ys[1:n])
    if (NROW(dt) != n) {
        stop('Invalid # of rows')
    }
    setattr(dt, 'sorted', 'value')
    dt
}

dt1 <- make_subdt(1)
dt2 <- make_subdt(2)
dt3 <- make_subdt(3)
dt4 <- make_subdt(4)

n_tests <- 1000
make_test_ys <- function(index) {
    conv(sample(2:(n_samples_set[index] - 1), n_tests, replace=TRUE)) + data_delta / 4.0
}

test_ys1 <- make_test_ys(1)
test_ys2 <- make_test_ys(2)
test_ys3 <- make_test_ys(3)
test_ys4 <- make_test_ys(4)

exec <- function(dt, test_ys) {
    sapply(test_ys, function(y) { dt[.(y), roll = +Inf] })
}

result <- microbenchmark(
    exec(dt1, test_ys1),
    exec(dt2, test_ys2),
    exec(dt3, test_ys3),
    exec(dt4, test_ys4),
    times=10, control=list(warmup=10)
)

df <- as.data.frame(summary(result))
print(df$mean)
print(result)
autoplot(result)

n_tests <- 100
measure_functions <- function(index) {
    ordered_ys <- all_ys[1:n_samples_set[index]]
    unordered_ys <- sample(ordered_ys)
    test_ys <- make_test_ys(index)
    dt <- data.table(value=ordered_ys)
    setattr(dt, 'sorted', 'value')

    result <- microbenchmark(
        exec(dt, test_ys),
        sapply(test_ys1, function(y) { Closest(ordered_ys, y) }),
        sapply(test_ys1, function(y) { Closest(unordered_ys, y) }),
        times=100, control=list(warmup=10)
    )

    df <- as.data.frame(summary(result))
    print(df$mean)
    print(result)
    result
}

result <- measure_functions(1)
autoplot(result)
result <- measure_functions(2)
autoplot(result)
result <- measure_functions(3)
autoplot(result)
result <- measure_functions(4)
autoplot(result)
