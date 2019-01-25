## Cause the protection stack overflow
library(hash)
options(expressions = 5e5)
f<-function(h){print(h[["1"]])}
h=hash(keys=c(1:1000000),values=c(2:1000001))
f(h)
