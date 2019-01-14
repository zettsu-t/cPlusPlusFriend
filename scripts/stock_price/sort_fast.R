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

n_samples_set <- c(100, 1000, 10000, 100000)
all_ys <- conv(seq(1, max(n_samples_set), data_step))

make_test_ys <- function(index, n_tests) {
    conv(sample(2:(n_samples_set[index] - 1), n_tests, replace=TRUE)) + data_delta / 4.0
}

exec <- function(dt, test_ys) {
    sapply(test_ys, function(y) { dt[.(y), roll = +Inf] })
}

measure_datatables <- function() {
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
    test_ys1 <- make_test_ys(1, n_tests)
    test_ys2 <- make_test_ys(2, n_tests)
    test_ys3 <- make_test_ys(3, n_tests)
    test_ys4 <- make_test_ys(4, n_tests)

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
    result
}

result <- measure_datatables()
autoplot(result)

measure_functions <- function(index) {
    n_tests <- 1000
    ordered_ys <- all_ys[1:n_samples_set[index]]
    unordered_ys <- sample(ordered_ys)
    dt <- data.table(value=ordered_ys)
    setattr(dt, 'sorted', 'value')
    test_ys <- make_test_ys(index, n_tests)

    result <- microbenchmark(
        exec(dt, test_ys),
        sapply(test_ys, function(y) { Closest(ordered_ys, y) }),
        sapply(test_ys, function(y) { Closest(unordered_ys, y) }),
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
