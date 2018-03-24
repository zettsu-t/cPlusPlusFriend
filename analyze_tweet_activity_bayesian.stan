data {
  int<lower=0> N_morning;
  int<lower=0> N_night;
  real<lower=0> Y_morning[N_morning];
  real<lower=0> Y_night[N_night];
}

parameters {
  real<lower=0> mu_morning;
  real<lower=0> mu_night;
  real<lower=0> sigma_morning;
  real<lower=0> sigma_night;
}

transformed parameters {
  real difference;
  difference = mu_night - mu_morning;
}

model {
  for (n in 1:N_morning)
    Y_morning[n] ~ student_t(4, mu_morning, sigma_morning);
  for (n in 1:N_night)
    Y_night[n] ~ student_t(4, mu_night, sigma_night);
}
