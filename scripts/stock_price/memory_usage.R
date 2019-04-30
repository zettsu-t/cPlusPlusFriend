rm(list=ls())
gc()
p <- function() { print(memory.size()) }
f <- function(v) {
  p()
  v <- append(v, 2)
  p()
}
v <- rep(1, 134217728)
p()
f(v)
gc()
p()
