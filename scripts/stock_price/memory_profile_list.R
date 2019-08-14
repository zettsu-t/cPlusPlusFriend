library(purrr)

n=1000001
seq(1,n-1) %>% purrr::reduce(.f=function(s,x) if (x %% 2 == 0) (s+x) else s, .init=0)
