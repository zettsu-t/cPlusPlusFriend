library(ggplot2)
library(tibble)
library(data.table)
library(reshape2)

quick <- FALSE

n_sample <- 10000
nb_size_set <- c(1, 4, 16, 64)
nb_prob_set <- c(0.1, 0.2, 0.4, 0.7)
divider_set <- c(2, 4, 8)

if (quick) {
    n_sample <- 1000
    nb_size_set <- 5
    nb_prob_set <- 0.2
    divider_set <- 4
}

scale_negative_binomial <- function(n_sample, nb_size, nb_prob, divider, is_size_divided) {
    if (is_size_divided) {
        rowSums(replicate(divider, rnbinom(n_sample, size=nb_size/divider, prob=nb_prob)))
    } else {
        ## Convert a prob of negative binomial distributions
        ## to a theta of Gamma (of not NB) distributions
        ## theta : scale parameter, beta : rate parameter of Gamma distributions
        theta_gamma <- (1 - nb_prob) / nb_prob
        scaled_prob <- 1 / (1 + theta_gamma / divider)
        rowSums(replicate(divider, rnbinom(n_sample, size=nb_size, prob=scaled_prob)))
    }
}

scale_gamma <- function(n_sample, gamma_shape, gamma_scale, divider, is_size_divided) {
    if (is_size_divided) {
        rowSums(replicate(divider, rgamma(n_sample, shape=gamma_shape/divider, scale=gamma_scale)))
    } else {
        rowSums(replicate(divider, rgamma(n_sample, shape=gamma_shape, scale=gamma_scale/divider)))
    }
}

make_mean_var_label <- function(name, mu, sigma2) {
    paste0(name, 'mean=', sprintf('%3.2f', mu),' var=', sprintf('%3.2f', sigma2))
}

analyze_distributions <- function(nb_size, nb_prob, gamma_shape, gamma_scale, is_nb, y_one, y_combined) {
    if (is_nb) {
        expected_mean <- nb_size * (1 - nb_prob) / nb_prob
        expected_var <- expected_mean / nb_prob
    } else {
        expected_mean <- gamma_shape * gamma_scale
        expected_var <- gamma_shape * gamma_scale * gamma_scale
    }

    actual_one_mean <- mean(y_one)
    actual_one_var <- var(y_one)
    actual_combined_mean <- mean(y_combined)
    actual_combined_var <- var(y_combined)

    expected_label <- make_mean_var_label('Expected: ', expected_mean, expected_var)
    actual_one_label <- make_mean_var_label('One:', actual_one_mean, actual_one_var)
    actual_combined_label <- make_mean_var_label('Combined: ', actual_combined_mean, actual_combined_var)

    table_one <- tabulate(y_one)
    table_combined <- tabulate(y_combined)
    max_count <- max(length(table_one), length(table_combined))
    table_one <- append(table_one, rep(0, max_count - length(table_one)))
    table_combined <- append(table_combined, rep(0, max_count - length(table_combined)))

    df <- tibble(one=table_one, combined=table_combined)
    df$x <- 1:NROW(df)
    df <- melt(df, c('x'))
    list(df=df,
         expected_mean=expected_mean,
         expected_var=expected_var,
         expected_label=expected_label,
         actual_one_mean=actual_one_mean,
         actual_one_var=actual_one_var,
         actual_one_label=actual_one_label,
         actual_combined_mean=actual_combined_mean,
         actual_combined_var=actual_combined_var,
         actual_combined_label=actual_combined_label)
}

draw_distributions <- function(n_sample, nb_size, nb_prob, divider, is_size_divided, is_nb) {
    gamma_shape <- nb_size
    gamma_scale <- 1.0 / (1.0 / nb_prob - 1.0)
    base_color <- 'black'
    chart_colors <- c('royalblue', 'orange')

    if (is_size_divided) {
        chart_suffix <- 'alpha divided'
    } else {
        chart_suffix <- 'theta divided'
    }

    if (is_nb) {
        y_one <- rnbinom(n_sample, size=nb_size, prob=nb_prob)
        y_combined <- scale_negative_binomial(n_sample, nb_size, nb_prob, divider, is_size_divided)
        chart_title <- paste('size =', nb_size, 'prob =', nb_prob, 'divider =', divider, chart_suffix)
    } else {
        y_one <- rgamma(n_sample, shape=gamma_shape, scale=gamma_scale)
        y_combined <- scale_gamma(n_sample, gamma_shape, gamma_scale, divider, is_size_divided)
        chart_title <- paste('shape =', gamma_shape, 'scale =', sprintf('%3.2f', gamma_scale), 'divider =', divider, chart_suffix)
    }

    result <- analyze_distributions(nb_size, nb_prob, gamma_shape, gamma_scale, is_nb, y_one, y_combined)

    df <- result$df
    range_x <- range(df$x)
    x_min <- range_x[1]
    x_position <- (range_x[2] - x_min) * 0.25 + x_min
    range_y <- range(df$value)
    y_min <- range_y[1]
    y_range <- range_y[2] - y_min

    add_label <- function(g, y_relative_pos, label, color) {
        g + annotate('text', x=x_position, y=y_min + y_range * y_relative_pos,
                     label=label, color=color)
    }

    g <- ggplot(df, aes(x=x, y=value, color=variable))
    g <- g + geom_line(position = 'identity', size=1.5, alpha=0.6)
    g <- g + ggtitle(chart_title)
    g <- g + scale_color_manual(values=chart_colors)

    g <- g + geom_vline(xintercept=result$expected_mean, color=base_color, linetype='dashed')
    g <- g + geom_vline(xintercept=result$actual_one_mean, color=chart_colors[1], linetype='dashed')
    g <- g + geom_vline(xintercept=result$actual_combined_mean, color=chart_colors[2], linetype='dashed')
    g <- add_label(g, 0.3, result$expected_label, base_color)
    g <- add_label(g, 0.4, result$actual_one_label, chart_colors[1])
    g <- add_label(g, 0.5, result$actual_combined_label, chart_colors[2])
    g <- g + theme(legend.position = c(1.0, 1.0), legend.justification = c(1.0, 1.0),
                   legend.key.width=unit(4, "line"))
    plot(g)
}

for(is_size_divided in c(TRUE, FALSE)) {
    draw_distributions(n_sample, nb_size=32.0, nb_prob=0.3, divider=8, is_size_divided, FALSE)
}

for (nb_size in nb_size_set) {
    for (nb_prob in nb_prob_set) {
        for (divider in divider_set) {
            for(is_size_divided in c(TRUE, FALSE)) {
                draw_distributions(n_sample, nb_size, nb_prob, divider, is_size_divided, TRUE)
            }
        }
    }
}
