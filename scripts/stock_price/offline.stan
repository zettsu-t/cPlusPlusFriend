data {
  int<lower=1> K;
  int<lower=1> N;
  int CAT[N];
  vector[N] Y;
}

parameters {
  ordered[K] mu;
  real<lower=0> sd_set[K];
}

model {
  sd_set ~ exponential(1);
  for(i in 1:N) {
    target += normal_lpdf(Y[i] | mu[CAT[i]], sd_set[CAT[i]]);
  }
}
