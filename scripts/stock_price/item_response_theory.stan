// Based on
// http://norimune.net/2949
// http://kosugitti.sakura.ne.jp/wp/wp-content/uploads/2013/08/irtnote.pdf
// http://www.ieice-hbkb.org/files/S3/S3gun_11hen_03.pdf
data {
  int<lower=0> N;
  int<lower=0> K;
  int Y[N,K];
  real<lower=0, upper=1> chance;
}

parameters {
  vector<lower=0.0, upper=1.0>[K] difficulty;
  vector<lower=0>[K] discrimination;
  vector<lower=0, upper=1>[N] theta;
}

model {
  difficulty ~ uniform(0, 1);
  theta ~ normal(0.5, 0.2);
  discrimination ~ normal(16.0, 3.0);

  if (chance < 0.001) {
    for(i in 1:K) {
      Y[,i] ~ bernoulli_logit(discrimination[i] * (theta - difficulty[i]));
    }
  } else {
    for(i in 1:K) {
      Y[,i] ~ bernoulli(chance + (1.0 - chance) * inv_logit(discrimination[i] * (theta - difficulty[i])));
    }
  }
}
