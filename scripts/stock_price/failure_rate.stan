data {
  int N;
}

parameters {
  real<lower=0.0, upper=1.0> p;
}

model {
  target += binomial_lpmf(0 | N, p);
}
