if (as.numeric(version$major) > 3 || floor(as.numeric(version$minor)) > 5) {
    RNGkind(sample.kind="Rounding")
}
version
set.seed(1)
sample(1:100, 10)
