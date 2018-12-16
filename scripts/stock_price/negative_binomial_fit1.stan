data {
  int<lower=0> K;
  int<lower=0> N;
  int Y[N,K];
}

parameters {
  real<lower=0> alpha_base;
  real<lower=0> alpha_trend;
  real<lower=0> beta_base;
  real<lower=0> beta_trend;
}

model {
  for(k in 1:K) {
    real alpha;
    real beta;
    alpha = alpha_trend * k + alpha_base;
    beta = beta_trend * k + beta_base;
    Y[,k] ~ neg_binomial(alpha, beta);
  }
}
