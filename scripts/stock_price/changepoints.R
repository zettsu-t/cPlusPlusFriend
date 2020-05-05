library(changepoint)
library(dplyr)
library(extrafont)
library(genlasso)
library(ggplot2)
library(lubridate)
library(purrr)
library(readr)
library(readxl)
library(stringr)

g_title <- 'VIX (CBOE Volatility Index) and its changepoints : dashed=CPs by genlasso::fusedlasso1d, dotted=high abs(diff(daily value))'

## Bind adjacent days into one day
bind_adjacent <- function(xs, margin) {
    purrr::reduce(.x=sort(xs), .init=integer(), .f=function(acc, x) {
        if (NROW(acc) == 0) {
            ## A first element
            x
        } else if ((tail(acc, 1) + margin) > x) {
            ## Bind near xs into one
            acc
        } else {
            c(acc,x)
        }
    })
}

read_source <- function() {
    select_value <- function(input_df) {
        names(input_df) <- names(input_df) %>%
            stringr::str_to_lower() %>%
            stringr::str_replace('vix\\s*', '')

        input_df %>%
            dplyr::select(c('date', 'close'))
    }

    ## VIX (CBOE Volatility Index)
    ## Data Source
    ## http://www.cboe.com/products/vix-index-volatility/vix-options-and-futures/vix-index/vix-historical-data
    df_history <- readxl::read_xls('incoming/vixarchive.xls', sheet='OHLC',
                                   col_types=c('date', rep('numeric', 4)), skip=1, na='n/a') %>%
        select_value() %>%
        dplyr::mutate(date=lubridate::as_date(date))

    df_current <- readr::read_csv('incoming/vixcurrent.csv', skip=1) %>%
        select_value() %>%
        dplyr::mutate(date=lubridate::as_date(date, format='%m/%d/%Y', tz='America/Chicago'))

    ## Lasso cannot handle NAs
    df <- dplyr::bind_rows(df_history, df_current) %>%
        na.omit() %>%
        dplyr::mutate(close=log10(close))

    ## Exclude this year
    this_year <- 2020
    df <- df %>% dplyr::filter(lubridate::year(date) < this_year)

    ## Smoothing is harmful to detect change points.
    ## df$close <- predict(loess(df$close ~ df$day, span = 0.05))
    df$day <- 1:NROW(df)
    df
}

find_change_points <- function(df) {
    model <- genlasso::fusedlasso1d(y=df$close, pos=df$day, gamma=0, minlam=0)
    coef(model)
}

draw_change_points <- function(df, beta, margin, n_betas, n_diffs, up_only, out_filename) {
    raw_diffs <- diff(df$close)
    betas <- beta[, n_betas]
    df$beta <- betas
    beta_diffs <- diff(betas)

    if (!up_only) {
        raw_diffs <- abs(raw_diffs)
    }

    diffs <- bind_adjacent(sort(head(order(raw_diffs), n_diffs)), margin)
    abs_min_coef <- nth(rev(sort(abs(beta_diffs))), n_betas)
    cps_positive <- bind_adjacent(which(beta_diffs > abs_min_coef), margin)
    cps_negative <- bind_adjacent(which(beta_diffs < -abs_min_coef), margin)

    ## Offset lags
    diffs <- diffs + 1
    cps_positive <- cps_positive + 1
    cps_negative <- cps_negative + 1

    font_name <- 'Tahoma'
    png(filename=out_filename, width=1600, height=800)
    g <- ggplot()
    g <- g + geom_vline(xintercept=df$date[diffs], linetype='dotted', color='orchid', size=1, alpha=0.7)
    g <- g + geom_text(data=df[diffs,], aes(x=date, label=date,
                                            y=log10(((2 + ceiling(1:NROW(diffs))) %% 9) * 5 + 32)),
                       colour="orchid4", angle=30, vjust = -1, size=3)

    g <- g + geom_vline(xintercept=df$date[cps_positive], linetype='dashed', color='royalblue1', size=2, alpha=0.7)
    g <- g + geom_text(data=df[cps_positive,], aes(x=date, label=date,
                                          y=log10((ceiling(1:NROW(cps_positive)) %% 9) * 5 + 28)),
                       colour="royalblue2", angle=30, vjust = -1, size=4)

    if (!up_only) {
        g <- g + geom_vline(xintercept=df$date[cps_negative], linetype='dashed', color='royalblue3', size=1, alpha=0.7)
        g <- g + geom_text(data=df[cps_negative,], aes(x=date, label=date,
                                              y=log10((ceiling(1:NROW(cps_negative)) %% 9) * 5 + 28)),
                           colour="royalblue4", angle=30, vjust = -1, size=4)
    }

    g <- g + geom_line(data=df, aes(x=date, y=close), lwd=2, color='gray50')
    g <- g + geom_line(data=df, aes(x=date, y=beta), lwd=1, color='black')
    g <- g + labs(title=g_title, x='Year', y='log10 (VIX value : daily close)')
    g <- g + theme(text=element_text(family=font_name),
                   legend.position="top",
                   legend.text=element_text(size=20),
                   legend.title=element_blank(),
                   axis.text=element_text(family=font_name, size=20),
                   axis.title=element_text(family=font_name, size=20),
                   strip.text=element_text(family=font_name, size=20),
                   plot.title=element_text(family=font_name, size=20))
    plot(g)
    dev.off()
    NROW(cps_positive) + NROW(cps_negative)
}

df <- read_source()
model.coef <- find_change_points(df)

## Bind days in a week
margin <- 7
n_betas <- 75
n_diffs <- 75
n_cps <- draw_change_points(df=df, beta=model.coef$beta, margin=margin, n_betas=n_betas, n_diffs=n_diffs,
                            up_only=FALSE, out_filename='vix_cp.png')
