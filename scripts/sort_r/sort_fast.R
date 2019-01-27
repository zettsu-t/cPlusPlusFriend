## Find a nearest value in a vector
library(Rcpp)
library(BH)
library(data.table)
library(DescTools)
library(ggplot2)
library(microbenchmark)
library(plyr)
library(dplyr)

## Build C++ code
Sys.setenv('PKG_CXXFLAGS'='-std=gnu++14')
sourceCpp('sort_fast.cpp')

## Quick check
quick_check <- function() {
    unordered_xs <- c(93, 91, 97, 95)
    ordered_xs <- sort(unordered_xs)
    target_ys <- c(90, 92, 94, 96, 98)

    actual <- as.integer(unlist(sapply(target_ys, function(y) { DescTools::Closest(unordered_xs, y) })))
    expected <- c(91, 93, 91, 93, 95, 97, 95, 97)
    if (!all(actual == expected)) {
        stop('actual != expected')
    }

    actual <- as.integer(unlist(sapply(target_ys, function(y) { DescTools::Closest(ordered_xs, y) })))
    expected <- c(91, 91, 93, 93, 95, 95, 97, 97)
    if (!all(actual == expected)) {
        stop('actual != expected')
    }

    actual <- find_ceil_number(ordered_xs, target_ys)
    expected <- c(1, 2, 3, 4, 0)
    if (!all(actual == expected)) {
        stop('actual != expected')
    }

    actual <- find_floor_number(ordered_xs, target_ys)
    expected <- c(0, 1, 2, 3, 4)
    if (!all(actual == expected)) {
        stop('actual != expected')
    }

    ordered_str_xs <- sort(c('B', 'BC', 'D', 'DE'))
    target_str_xs <- sort(c('A', 'B', 'BA', 'C', 'CD', 'D', 'E'))
    actual <- find_ceil_string(ordered_str_xs, target_str_xs)
    expected <- c(1, 1, 2, 3, 3, 3, 0)
    if (!all(actual == expected)) {
        stop('actual != expected')
    }

    ordered_str_xs <- rev(ordered_str_xs)
    actual <- find_floor_reverse_string(ordered_str_xs, target_str_xs)
    expected <- c(0, 1, 1, 2, 2, 3, 4)
    expected <- sapply(expected, function(x) ifelse(x == 0, 0, length(ordered_str_xs) + 1 - x))
    if (!all(actual == expected)) {
        stop('actual != expected')
    }

    print('Quick check passed')
}

quick_check()

long_check <- function(n_sample, n_trial, ordered, use_r, no_cpp_serial, times) {
    data_step <- 0.01
    half_step <- data_step / 2.0
    min_x <- 1.0 + data_step
    max_x <- 1.0 + data_step * n_sample

    ordered_xs <- seq(min_x, max_x, length.out=n_sample)
    target_ys <- seq(min_x - half_step, max_x + half_step, length.out=n_trial)
    if (ordered) {
        xs <- ordered_xs
        if (any(diff(xs) <= 0)) {
            stop('Must be ordered')
        }
    } else {
        xs <- sample(ordered_xs)
        if (all(diff(xs) > 0)) {
            stop('Must be unordered')
        }
    }

    r_serial <- NULL
    if (use_r) {
        r_serial <- microbenchmark(
            sapply(target_ys, function(y) { DescTools::Closest(xs, y) }),
            times=times)
    }

    cpp_vec <- NULL
    cpp_serial <- NULL
    if (ordered) {
        cpp_vec <- microbenchmark(
            find_ceil_number(xs, target_ys),
            times=times)

        if (!no_cpp_serial) {
            cpp_serial <- microbenchmark(
                sapply(target_ys, function(y) { find_ceil_number(xs, y) }),
                times=times)
        }
    }

    return (list(r_serial=r_serial, cpp_vec=cpp_vec, cpp_serial=cpp_serial))
}

draw_time <- function(arg_df, title) {
    df <- arg_df
    df$time <- pmax(df$time, 1)
    g <- ggplot(df, aes(x=n, y=time, color=method, fill=method))
    g <- g + geom_boxplot(position=position_dodge(1), outlier.shape = NA)
    g <- g + scale_y_log10(limits=quantile(df$time, c(0.1, 0.9)))
    g <- g + labs(title=title)
    g <- g + xlab('Size of numbers')
    g <- g + ylab('Time [nanosecond]')
    plot(g)

    df.median <- df %>% group_by(n, method) %>% summarise(median = median(time, na.rm = TRUE))
    g <- ggplot(df.median)
    g <- g + geom_line(aes(x=n, y=median, group=method, color=method))
    g <- g + labs(title=title)
    g <- g + xlab('Size of numbers')
    g <- g + ylab('Time [nanosecond] : median')
    g <- g + ylim(0, max(df.median$median) * 1.5)
    plot(g)
}

n_trial <- 1000
times <- 100
df_ordered <- lapply(c(10, 32, 100, 316, 1000, 3162, 10000), function(n_sample) {
    result <- long_check(n_sample=n_sample, n_trial=n_trial, ordered=TRUE, use_r=TRUE, no_cpp_serial=FALSE, times=times)
    len_r_serial <- length(result$r_serial$time)
    len_cpp_vec <- length(result$cpp_vec$time)
    len_cpp_serial <- length(result$cpp_serial$time)

    n_str <- as.character(n_sample)
    rbind(data.frame(n=rep(n_str, len_r_serial), method=rep('R DescTools::Closest', len_r_serial), time=result$r_serial$time),
          data.frame(n=rep(n_str, len_cpp_serial), method=rep('C++ std::lower_bound (forward)', len_cpp_serial), time=result$cpp_serial$time))
}) %>% ldply(data.table)

draw_time(df_ordered, 'Time to find the nearest ceil numbers (ordered)')


df_cpp_large <- lapply(1:7, function(i) {
    n_sample <- as.integer(10^i)
    result <- long_check(n_sample=n_sample, n_trial=n_trial, ordered=TRUE, use_r=FALSE, no_cpp_serial=TRUE, times=times)
    len_cpp_vec <- length(result$cpp_vec$time)
    len_cpp_serial <- length(result$cpp_serial$time)
    n_str <- sprintf('%d', n_sample)
    data.frame(n=rep(n_str, len_cpp_vec), method=rep('CPP std::lower_bound (forward)', len_cpp_vec), time=result$cpp_vec$time)
}) %>% ldply(data.table)

draw_time(df_cpp_large, 'Time to find the nearest ceil numbers (std::lower_bound)')


df_r <- lapply(c(10, 32, 100, 316, 1000, 3162, 10000), function(n_sample) {
    result_ordered <- long_check(n_sample=n_sample, n_trial=n_trial, ordered=TRUE, use_r=TRUE, no_cpp_serial=FALSE, times=times)
    result_unordered <- long_check(n_sample=n_sample, n_trial=n_trial, ordered=FALSE, use_r=TRUE, no_cpp_serial=FALSE, times=times)
    len_ordered <- length(result_ordered$r_serial$time)
    len_unordered <- length(result_unordered$r_serial$time)
    n_str <- as.character(n_sample)
    rbind(data.frame(n=rep(n_str, len_ordered), method=rep('R ordered', len_ordered), time=result_ordered$r_serial$time),
          data.frame(n=rep(n_str, len_unordered), method=rep('R unordered', len_unordered), time=result_unordered$r_serial$time))
}) %>% ldply(data.table)

draw_time(df_r, 'Time to find the nearest ceil numbers (unordered)')


df_cpp <- lapply(1:7, function(i) {
    n_sample <- as.integer(10^i)
    result <- long_check(n_sample=n_sample, n_trial=n_trial, ordered=TRUE, use_r=FALSE, no_cpp_serial=FALSE, times=times)
    len_cpp_vec <- length(result$cpp_vec$time)
    len_cpp_serial <- length(result$cpp_serial$time)
    n_str <- sprintf('%d', n_sample)
    rbind(data.frame(n=rep(n_str, len_cpp_vec), method=rep('C++ passing vector', len_cpp_vec), time=result$cpp_vec$time),
          data.frame(n=rep(n_str, len_cpp_serial), method=rep('C++ calling serial', len_cpp_serial), time=result$cpp_serial$time))
}) %>% ldply(data.table)

draw_time(df_cpp, 'Time to find the nearest ceil numbers (std::lower_bound)')
