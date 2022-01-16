# C++ 20 sample

## Environment

``` bash
docker build . -t rcpp20
```

## Example

Build and check this package on RStudio.

``` r
library(rcpp20popcount)
rcpp20popcount::popcount(0:8)
```

## Testing

You can test R code and C++ code.

``` r
devtools::test()
```

``` bash
cd tests
mkdir build
cd build
cmake ..
make
make test
```
