library(assertthat)
library(data.table)
library(stringr)
library(plyr)
library(dplyr)
library(rstan)
library(ggmcmc)

options(warn = 1)

## Reads a CSV file and stores it into a data.frame and a data.table
filename <- 'data_frame_table.csv'
df_file <- read.csv(filename)
dt_file <- fread(filename)

## Prints classes of each frame
df <- df_file
dt <- dt_file
print(class(df))
print(class(dt))

## Prints classes of columns
## In default, read.csv treats strings (characters) as factors.
## Both data.frames and data.tables have a logical type for NA-only columns.
print_column_types <- function(df, dt) {
    f <- function (d) {
        sapply(names(d), function(name) { print(class(d[[name]])) })
    }
    sapply(names(df), f)
    sapply(names(df), f)
}

print_column_types(df, dt)


## Puts a typed vector to an NA-only column.
v <- c(1,2,3)
df$na_value <- v
dt$na_value <- v
print(df$na_value)
print(dt$na_value)
print_column_types(df, dt)


## Binds a typed vector to an NA-only column.
df_alt <- data.frame(
    int_value=c(4, 5),
    float_value=c(5.4, 5.5),
    str_value=c('def', 'efg'),
    na_value=c(14, 15),
    bool_value=c(TRUE, FALSE))

dt_alt <- data.table(
    int_value=c(4, 5),
    float_value=c(5.4, 5.5),
    str_value=c('def', 'efg'),
    na_value=c(14, 15),
    bool_value=c(TRUE, FALSE))

df <- df_file
dt <- dt_file
df_bound <- rbind(df, df_alt)
print(df_bound$na_value)
dt_bound <- rbind(dt, dt_alt)
print(dt_bound$na_value)
print_column_types(df, dt)

df <- df_file
dt <- dt_file
df_bound <- dplyr::bind_rows(df, df_alt)
print(df_bound$na_value)
dt_bound <- dplyr::bind_rows(dt, dt_alt)
print(dt_bound$na_value)
print_column_types(df, dt)


## Binds a data.frame and another data.frame with a missing column
df_top <- data.frame(
    int_value=integer(),
    float_value=numeric(),
    str_value=character(),
    bool_value=logical())

dt_top <- data.table(
    int_value=integer(),
    float_value=numeric(),
    str_value=character(),
    bool_value=logical())

df <- df_file
dt <- dt_file
df_bound <- rbind(df_top, df)
print(df_bound$na_value)

## Causes an error due to a missing column
try({
    dt_bound <- rbind(dt_top, dt)
    print(class(dt_bound$na_value))
})

## Requires fill=TRUE
dt_bound <- rbind(dt_top, dt, fill=TRUE)
print(dt_bound$na_value)
print_column_types(df, dt)

df <- df_file
dt <- dt_file
df_bound <- dplyr::bind_rows(df_top, df)
print(df_bound$na_value)
dt_bound <- dplyr::bind_rows(dt_top, dt)
print(dt_bound$na_value)
print_column_types(df, dt)


## Selects columns
df <- df_file
dt <- dt_file

name <- 'int_value'
df[,name]
try({print(dt[,name])})
try({print(df[,..name])})
dt[,..name]
df[[name]]
dt[[name]]


## Selects cells
df_cell <- df[1, 'int_value']
class(df_cell)
dt_cell <- dt[1, 'int_value']
class(dt_cell)
class(unlist(dt_cell))

df_cell <- df[1:2, 'int_value']
class(df_cell)
dt_cell <- dt[1:2, 'int_value']
class(dt_cell)
class(unlist(dt_cell))


## Selects rows
check_row_is_unique <- function(result) {
    print(result)
    assert_that(NROW(result) == 1)
}

check_row_is_unique(df[df$str_value == 'abc',])
check_row_is_unique(dt[dt$str_value == 'abc',])
check_row_is_unique(df %>% filter(str_value == 'abc'))
check_row_is_unique(dt %>% filter(str_value == 'abc'))

str_value <- 'abc'
check_row_is_unique(df[df$str_value == str_value,])
## These expressions evaluate d*$str_value == d*$str_value and
## do not evaluate str_value as a column == str_value as a variable.
try({check_row_is_unique(dt[dt$str_value == str_value,])})
try({check_row_is_unique(df %>% filter(str_value == str_value))})
try({check_row_is_unique(dt %>% filter(str_value == str_value))})

## These are what we surely expect
str_v <- 'abc'
check_row_is_unique(dt[dt$str_value == str_v,])
check_row_is_unique(df %>% filter(str_value == str_v))
check_row_is_unique(dt %>% filter(str_value == str_v))

## !! is a syntactic sugar of UQ and notice that !! requires a surrounding ()
## https://stackoverflow.com/questions/27197617/filter-data-frame-by-character-column-name-in-dplyr
name <- 'str_value'
check_row_is_unique(df %>% dplyr::filter(UQ(as.name(name)) == str_v))
check_row_is_unique(dt %>% dplyr::filter(UQ(as.name(name)) == str_v))
check_row_is_unique(df %>% dplyr::filter((!!(as.name(name))) == str_v))
check_row_is_unique(dt %>% dplyr::filter((!!(as.name(name))) == str_v))
try({check_row_is_unique(df %>% dplyr::filter(!!(as.name(name)) == str_v))})
try({check_row_is_unique(dt %>% dplyr::filter(!!(as.name(name)) == str_v))})


## Do not use 'key' as a column name because key is a keyword parameter of data.table
try({data.table(key=c(1,2,3), value=c(4,5,6))})
## This is a workaround of it.
dt_local <- data.table(dummy=c(1,2,3), value=c(4,5,6))
dt_local %>% rename(key=dummy)


## Copies columns by their names
df_left_base <- data.frame(a=c(1,2,3), b=c(4,5,6), d=c(7,8,9))
df_right_base <- data.frame(a=c(11,12,13), b=c(14,15,16))
dt_left_base <- data.table(a=c(1,2,3), b=c(4,5,6), d=c(7,8,9))
dt_right_base <- data.table(a=c(11,12,13), b=c(14,15,16))

df_left <- df_left_base
df_right <- df_right_base
df_left[,names(df_right)] <- df_right[,names(df_right)]
df_left

dt_left <- dt_left_base
dt_right <- dt_right_base
dt_left <- data.table(a=c(1,2,3), b=c(4,5,6), d=c(7,8,9))
dt_right <- data.table(a=c(11,12,13), b=c(14,15,16))
try({
    dt_left[,names(dt_right)] <- dt_right[,names(dt_right)]
})

dt_left <- dt_left_base
dt_right <- dt_right_base
invisible(sapply(names(dt_right), function (name) { dt_left[[name]] <<- dt_right[[name]] }))
dt_left

## data.frames can take [] in row selection but data.tables do not
df_left <- df_left_base
df_left['a'] <- c(21, 22, 23)
df_left
df_left[['a']] <- c(31, 32, 33)
df_left

dt_left <- dt_left_base
try({dt_left['a'] <- c(21, 22, 23)})
dt_left
dt_left[['a']] <- c(31, 32, 33)
dt_left


## Testing to keep row names
options(mc.cores = parallel::detectCores())
rstan_options(auto_write = TRUE)
Sys.setenv(LOCAL_CPPFLAGS = '-march=native')

n <- 10000
theta <- 0.5
mu <- 0.1
positives <- sample(x=c(1,0), size=n, replace=T, prob=c(mu, 1.0-mu))
first_coins <- sample(x=c(1,0), size=n, replace=T, prob=c(theta, 1.0-theta))
second_coins <- sample(x=c(1,0), size=n, replace=T, prob=c(theta, 1.0-theta))
sample_df <- data.table(first_coin=first_coins, second_coin=second_coins, positive=positives)
sample_df$answer <- apply(sample_df, 1, function(x) {
    ifelse((x['first_coin'] > 0 || x['positive'] > 0) &&
           (x['first_coin'] == 0 || x['second_coin'] > 0), 1, 0)})
input_data <- list(N=nrow(sample_df), Y=sample_df$answer, theta=theta)
fit.stan <- stan(file='data_frame_table.stan', data=input_data,
                 iter=1000, warmup=500, chains=4, seed=123)
traceplot(fit.stan)
ggmcmc(ggs(fit.stan))

result.stan <- get_posterior_mean(fit.stan)
result_df <- data.frame(result.stan)
result_dt <- data.table(result.stan)
result_df['mu',]
try({print(result_dt['mu',])})

## Needs explicit keep.rownames to preserve row names
result_dt <- data.table(result.stan, keep.rownames=TRUE)
## Cannot use a data.frame way to select rows
try({print(result_dt['mu',])})
result_dt %>% filter(rn == 'mu')
