// Algorithm Based on
// https://github.com/CamDavidsonPilon/Probabilistic-Programming-and-Bayesian-Methods-for-Hackers/blob/master/Chapter2_MorePyMC/Ch2_MorePyMC_PyMC3.ipynb
// And see https://github.com/MatsuuraKentaro/RStanBook/blob/master/chap11/model/model11-1.stan
data {
  int<lower=0> N;
  int Y[N];
  real theta;
}

parameters {
  real<lower=0, upper=1> mu;
}

model {
  for (i in 1:N) {
    real log_p[2];
    log_p[1] = bernoulli_lpmf(1 | theta) + bernoulli_lpmf(Y[i] | mu);
    log_p[2] = bernoulli_lpmf(0 | theta) + bernoulli_lpmf(Y[i] | theta);
    target += log_sum_exp(log_p);
  }
}
