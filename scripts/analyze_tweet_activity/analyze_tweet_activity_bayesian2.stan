data {
  int<lower=1> N_base;
  vector<lower=0>[N_base] Y_base;
  int<lower=1> K;
  int<lower=1> N;
  int<lower=1> TID[N];
  vector<lower=0>[N] Y;
}

parameters {
  real<lower=0> mu_base;
  real<lower=0> sigma_base;
  vector<lower=0>[K] mu;
  vector<lower=0>[K] sigma;
}

transformed parameters {
  vector[K] difference;
  difference[1:K] = mu[1:K] - mu_base;
}

model {
  Y_base ~ student_t(4, mu_base, sigma_base);
  for (i in 1:N) {
    Y[i] ~ student_t(4, mu[TID[i]], sigma[TID[i]]);
  }
}
