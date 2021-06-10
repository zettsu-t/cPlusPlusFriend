n <- 100000000
values <- floor(cumsum(1/seq_len(n)))

## 0-based indexing like C++
positions <- c(0, 1, 1 + which(diff(values) == 1))
paste(positions, collapse = ", ")

## 0, 1, 4, 11, 31, 83, 227, 616, 1674, 4550, 12367, 33617, 91380, 248397, 675214, 1835421, 4989191, 13562027, 36865412
