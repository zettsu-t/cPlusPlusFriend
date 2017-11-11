library(purrr)
library(rBayesianOptimization)

secretary_problem <- function (num_items, num_pass) {
    nums <- sample(seq(1, num_items, 1))
    pivot <- max(nums[1:num_pass])
    tail_nums <- nums[(num_pass+1):num_items]

    num = detect(tail_nums, function(x) {x > pivot})
    selected_num <- if (is.null(num)) nums[num_items] else num
}

simulate_secretary_problem <- function (num_items, num_pass, trial_size) {
    selected_nums <- replicate(trial_size, secretary_problem(num_items, num_pass))
    m <- Reduce(function(sum, num) sum + if (num_items == num) 1.0 else 0.0, x=selected_nums, init = 0.0)
    m / trial_size
}

optimize_secretary_problem <- function (num_items, trial_size, iter_size) {
    sim <- function (cutoff_ratio) {
        num_pass = round(num_items * cutoff_ratio)
        score = if (num_pass <= 1 || num_pass >= num_items) {
            1.0 / num_items
        } else {
            simulate_secretary_problem(num_items, num_pass, trial_size)
        }
        list(Score = score, Pred = 0)
    }

    result <- BayesianOptimization(sim,
                                   bounds = list(cutoff_ratio = c(0.0, 1.0)),
                                   init_points = 10, n_iter = iter_size,
                                   acq = 'ei', kappa = 2.576,
                                   eps = 0.0, verbose = TRUE)
}

optimize_secretary_problem(100, 10000, 50)
