library(plyr)
library(dplyr)
library(purrr)
library(purrrlyr)
library(reshape2)
library(tibble)
library(ggplot2)
library(gplots)
library(RColorBrewer)

## Split 1:size to ranges [i:i+step-1]
make_range_seq <- function(size, step) {
    starts <- seq(1, size, step)
    ends <- c(tail(starts, -1) - 1, size)
    tibble(start=starts, end=ends)
}

## Convolute n_col_step * n_row_step rectangulars with a kernel
convolute_matrix <- function(mat, n_col_step, n_row_step) {
    purrrlyr::by_row(make_range_seq(NCOL(mat), n_col_step), function(col) {
        purrrlyr::by_row(make_range_seq(NROW(mat), n_row_step), function(row) {
            ## Kernel
            sum(mat[row$start:row$end, col$start:col$end])
        })$.out %>% ldply()
    }) $.out %>% bind_cols()
}

## Convolute recursively
convolute_to_var_mean <- function(mat, n_step, step_width) {
    new_mat <- mat
    sapply(1:n_step, function(i) {
        new_mat <<- as.matrix(convolute_matrix(new_mat, step_width, step_width))
        elements <- as.vector(new_mat)
        mean_elements <- mean(elements)
        var_elements <- var(elements)
        var_elements / mean_elements
    })
}

get_2_steps <- function(n_row, n_col) {
    n_size <- n_row * n_col
    step_width <- 2
    ## Used log_{step_width}() if step_width != 0
    n_step <- floor(log2(min(n_row, n_col))) - 1
    list(n_size=n_size, step_width=step_width, n_step=n_step)
}

pure_pois <- function(n_row, n_col, arg_mu) {
    param <- get_2_steps(n_row, n_col)
    n_size <- param$n_size
    step_width <- param$step_width
    n_step <- param$n_step

    mat <- matrix(rpois(n=n_size, lambda=arg_mu), nrow=n_row)
    convolute_to_var_mean(mat, n_step, step_width)
}

pure_lognorm <- function(n_row, n_col, arg_mu, arg_sd) {
    param <- get_2_steps(n_row, n_col)
    n_size <- param$n_size
    step_width <- param$step_width
    n_step <- param$n_step

    mat <- matrix(exp(rnorm(n=n_size, mean=arg_mu, sd=arg_sd)), nrow=n_row)
    convolute_to_var_mean(mat, n_step, step_width)
}

ramdom_pois_lognorm <- function(n_row, n_col, arg_mu, arg_sd) {
    param <- get_2_steps(n_row, n_col)
    n_size <- param$n_size
    step_width <- param$step_width
    n_step <- param$n_step

    rand_set <- exp(rnorm(n=n_size, mean=arg_mu, sd=arg_sd))
    mat <- matrix(sapply(rand_set, function(x) { rpois(n=1, lambda=x) }), nrow=n_row)
    convolute_to_var_mean(mat, n_step, step_width)
}

fill_zigzag <- function(n_row, n_col, arg_mu, arg_sd) {
    n_size <- n_row * n_col
    x <- 1
    y <- 1

    mat <- matrix(rep(x=0, times=n_size), nrow=n_row)
    sapply(1:n_size, function(i) {
        m <- 0
        nextx <- x - 1
        nexty <- y + 1
        if ((x == 1) && (y == 1)) {
            m <- 0
            nextx <- 2
            nexty <- 1
        } else if (y == 1) {
            m <- mat[y, x - 1]
        } else if ((x == 1) || (y == n_row)){
            m <- mat[y - 1, x]
            nextx <- min(y + 1, n_col)
            nexty <- x + y + 1 - nextx
        } else {
            m <- (mat[y, x - 1] + mat[y - 1, x]) / 2.0
        }
        v <- rnorm(n=1, mean=m, sd=arg_sd)
        mat[y, x] <<- v
        x <<- nextx
        y <<- nexty
        v
    })

    mat
}

random_lognorm <- function(n_row, n_col, arg_mu, arg_sd) {
    param <- get_2_steps(n_row, n_col)
    step_width <- param$step_width
    n_step <- param$n_step

    base_mat <- fill_zigzag(n_row=n_row, n_col=n_col, arg_mu=arg_mu, arg_sd=arg_sd)
    mat <- matrix(sapply(exp(base_mat), function(x) { rpois(n=1, lambda=x) }), nrow=n_row)
    var_mean <- convolute_to_var_mean(mat, n_step, step_width)
    list(var_mean=var_mean, mat=base_mat)
}

size <- 512
pure_pois(n_row=size, n_col=size, arg_mu=5.0)
pure_lognorm(n_row=size, n_col=size, arg_mu=1.0, arg_sd=0.2)
ramdom_pois_lognorm(n_row=size, n_col=size, arg_mu=0.5, arg_sd=0.5)
result <- random_lognorm(n_row=size, n_col=size, arg_mu=1.0, arg_sd=0.001)
print(result$var_mean)

colMain <- colorRampPalette(brewer.pal(8, "Blues"))(20)
heatmap.2(result$mat, dendrogram='none', Rowv=FALSE, Colv=FALSE, trace='none',col=colMain)
heatmap.2(margins=c(2,2), result$mat, dendrogram='none', key=FALSE, density.info="none", labRow=FALSE, labCol=FALSE, Rowv=FALSE, Colv=FALSE, trace='none',col=colMain)
