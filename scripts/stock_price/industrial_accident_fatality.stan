// Based on
// http://nowave.it/pages/bayesian-changepoint-detection-with-r-and-stan.html
data {
  int<lower=0> T;
  int X[T];
  vector[T] Y;
}

parameters {
  real<lower=0> mu_e;
  real<lower=0> sigma_e;
  real<lower=0> mu_l;
  real trend_l;
  real<lower=0> sigma_l;
}

transformed parameters {
      vector[T] log_p;
      real mu_base;
      real mu;
      real sigma;
      real trend;
      real head_year;
      log_p = rep_vector(-log(T), T);
      for (tau in 1:T)
        for (n in 1:T) {
          mu_base = n < tau ? mu_e : mu_l;
          sigma = n < tau ? sigma_e : sigma_l;
          trend = n < tau ? 0 : trend_l;
          head_year = n < tau ? X[1] : tau;
          mu = mu_base + (n - head_year) * trend;
          log_p[tau] = log_p[tau] + normal_lpdf(Y[n] | mu, sigma);
      }
}

model {
    mu_e ~ normal(0, 10000);
    mu_l ~ normal(0, 10000);
    sigma_e ~ cauchy(0, 1000);
    sigma_l ~ cauchy(0, 100);
    trend_l ~ normal(0, 500);
    target += log_sum_exp(log_p);
}

generated quantities {
    int<lower=1,upper=T> tau;
    simplex[T] sp;
    sp = softmax(log_p);
    tau = categorical_rng(sp);
}
