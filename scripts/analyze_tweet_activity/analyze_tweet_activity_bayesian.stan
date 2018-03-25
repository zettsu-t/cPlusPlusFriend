data {
  int<lower=0> N_base;
  int<lower=0> N_alt;
  vector[N_base] Y_base;
  vector[N_alt] Y_alt;
}

parameters {
  real<lower=0> mu_base;
  real<lower=0> mu_alt;
  real<lower=0> sigma_base;
  real<lower=0> sigma_alt;
}

transformed parameters {
  real difference;
  difference = mu_alt - mu_base;
}

model {
  Y_base ~ student_t(4, mu_base, sigma_base);
  Y_alt ~ student_t(4, mu_alt, sigma_alt);
}
