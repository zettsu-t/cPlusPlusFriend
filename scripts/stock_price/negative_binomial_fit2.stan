data {
  int<lower=0> K;
  int<lower=0> N;
  int Y[N,K];
}

parameters {
  real<lower=0> mu_base;
  real<lower=0> mu_trend;
  real<lower=0> variance_base;
  real<lower=0> variance_trend;
}

model {
  for(k in 1:K) {
    real mu;
    real variance_nb;
    real phi;
    mu = mu_trend * k + mu_base;
    // sigma must be greater than mu!
    variance_nb = (1.0 + variance_trend) * mu + variance_base;
    phi = (mu * mu) / (variance_nb - mu);
    Y[,k] ~ neg_binomial_2(mu, phi);
  }
}
