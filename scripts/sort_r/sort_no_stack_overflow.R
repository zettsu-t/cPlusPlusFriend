## Avoid the protection stack overflow
library(Rcpp)
library(BH)

## Build C++ code
Sys.setenv('PKG_CXXFLAGS'='-std=gnu++14')
sourceCpp('sort_no_stack_overflow.cpp')

clear_table(0)
n <- 100
nums <- sample(1:n, replace=FALSE)
keys <- sprintf('%04d', nums)
values <- sprintf('%05d', nums * 3)
print(head(keys))
add_table(keys, values)
df <- convert_table_to_dataframe(0)
print(head(df))

n <- 10000000
clear_table(0)
keys <- as.character(c(1:n))
values <- as.character(c(2:(n+1)))
add_table(keys, values)

n_trial <- 10000
nums <- c(1, n, sample(1:n, n_trial - 2, replace=TRUE))
keys <- sprintf("%d", nums)
expected <- as.character(nums + 1)
actual <- find_table(keys)
if (!all(expected == actual)) {
    stop("Test failed")
}

keys <- c(sprintf("%d", c(-1, 0, n+1, n+2), "x", "1e-5", ""))
expected <- rep("", length(keys))
actual <- find_table(keys)
if (!all(expected == actual)) {
    stop("Test failed")
}
