## Cause the protection stack overflow
library(hash)
options(expressions = 5e5)
f<-function(h){print(h[["1"]])}
h=hash(keys=c(1:1000000), values=c(2:1000001))
f(h)

s <- 1
n <- 100002
ns <- s:n
keys <- as.character(ns)
values <- as.character(ns + 1)
h=hash(keys=keys, values=values)
result <- sapply(s:n, function(x) {
    h[[as.character(x)]] == sprintf("%d", x+1)
})
keys[which(!result)]
values[which(!result)]
