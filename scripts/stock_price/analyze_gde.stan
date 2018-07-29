data {
  int<lower=0> T;
  int<lower=0> N;
  vector[N] X;
}

parameters {
  real<lower=0> sigma;
  vector[T] weight;  // weight of T..1 terms prior values
}

model {
  for (i in 1:N-T) {
    real s = dot_product(weight, X[i:i+T-1]);
    for (t in 1:T) {
      target += normal_lpdf(X[i+T] | s, sigma);
    }
  }
}
