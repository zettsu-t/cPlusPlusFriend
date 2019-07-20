library(purrr)
args <- commandArgs(trailingOnly=TRUE)
vec_size <- ifelse(length(args) >= 1, as.integer(args[1]), 250000)
df <- 6
lines <- paste0('std::vector<double> data {',
                paste(purrr::map(rt(vec_size, df), function(x) { sprintf('%.6f', x) }),
                      sep=',', collapse=','), '};\n\n')

fileConn <- file('axes_compile_time.inc')
writeLines(lines, fileConn)
close(fileConn)
