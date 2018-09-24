data {
  int<lower=0> T;
  int<lower=0> W;
  vector[T] Y;
  int E[T];
}

parameters {
  vector[7] mu_week;
  vector<lower=0>[7] sigma_week;
  real mu_event;
  real<lower=0> sigma_event;
  vector[T] event_visitors;
}

transformed parameters {
  vector[T] mu_visitors;
  for (w in 1:W) {
    for (d in 1:7) {
      int index = (w - 1) * 7 + d;
      if (E[index] == 0) {
        mu_visitors[index] = mu_week[d];
      } else {
        mu_visitors[index] = mu_week[d] + mu_event * exp(event_visitors[index]);
      }
    }
  }
}

model {
  for (w in 1:W) {
    for (d in 1:7) {
      int index = (w - 1) * 7 + d;
      if (E[index] == 0) {
        target += normal_lpdf(event_visitors[index] | 0, 0.0001);
      } else {
        target += normal_lpdf(event_visitors[index] | 0, sigma_event);
      }
      target += normal_lpdf(Y[index] | mu_visitors[index], sigma_week[d]);
    }
  }
}
