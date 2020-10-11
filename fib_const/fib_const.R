library(purrr)
purrr::reduce(.x=1:11, .init=c(), .f=function(v, i) {
    if (NROW(v) == 0) {
        c(0)
    } else if (NROW(v) == 1) {
        c(v, 1)
    } else{
        c(v, sum(tail(v, 2)))
    }
})
