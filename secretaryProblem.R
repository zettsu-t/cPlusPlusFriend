# This Question is cited from the book
# Dennis E. Shasha, "Puzzles for Programmers and Pros", 2007, Wrox
library(purrr)
library(rBayesianOptimization)

secretary_problem <- function (item_size, pass_list) {
    nums <- sample(seq(1, item_size, 1))
    pivot <- max(nums[1:pass_list[1]])
    selected_nums <- c()
    index_left <- pass_list[1] + 1

    for (pass_index in seq(1, length(pass_list), 1)) {
        index_right <- if (pass_index >= length(pass_list)) item_size else pass_list[pass_index + 1]
        index_next_left <- index_right + 1
        while((length(selected_nums) < pass_index) && ((index_left + 1) < index_right)) {
            sliced_nums <- nums[index_left:index_right]
            index <- detect_index(sliced_nums, function(x) {x > pivot})
            if (index <= 0) {
                break
            }
            pivot <- sliced_nums[index]
            selected_nums <- append(selected_nums, pivot)
            index_left <- index_left + index
        }
        index_left <- index_next_left
    }

    if (length(selected_nums) == 0) nums[item_size] else max(selected_nums)
}

simulate_secretary_problem <- function (item_size, pass_list, trial_size) {
    selected_nums <- replicate(trial_size, secretary_problem(item_size, pass_list))
    m <- Reduce(function(sum, num) sum + if (item_size == num) 1.0 else 0.0, x=selected_nums, init = 0.0)
    m / trial_size
}

optimize_secretary_problem_1 <- function (item_size, trial_size, iter_size) {
    sim <- function (cutoff) {
        pass_list <- c(cutoff) * item_size
        score <- if (pass_list[1] <= 1 || pass_list[1] >= item_size) {
            1.0 / item_size
        } else {
            simulate_secretary_problem(item_size, pass_list, trial_size)
        }
        list(Score = score, Pred = 0)
    }

    result <- BayesianOptimization(sim,
                                   bounds = list(cutoff = c(0.0, 1.0)),
                                   init_points = 10, n_iter = iter_size,
                                   acq = 'ei', kappa = 2.576,
                                   eps = 0.0, verbose = TRUE)
}

optimize_secretary_problem_3 <- function (item_size, trial_size, iter_size) {
    sim <- function (cutoff_1, cutoff_2, cutoff_3) {
        pass_list <- round(c(cutoff_1, cutoff_2, cutoff_3) * item_size)
        score <- if (pass_list[1] >= pass_list[2] || pass_list[2] >= pass_list[3] ||
                    pass_list[1] <= 1 || pass_list[3] >= item_size) {
            1.0 / item_size
        } else {
            simulate_secretary_problem(item_size, pass_list, trial_size)
        }
        list(Score = score, Pred = 0)
    }

    result <- BayesianOptimization(sim,
                                   bounds = list(cutoff_1 = c(0.0, 1.0), cutoff_2 = c(0.0, 1.0), cutoff_3 = c(0.0, 1.0)),
                                   init_points = 120, n_iter = iter_size,
                                   acq = 'ei', kappa = 2.576,
                                   eps = 0.0, verbose = TRUE)
}

# Optimal shown in the book
simulate_secretary_problem(100, c(14,32,64), 40000)

# Explore solutions
optimize_secretary_problem_1(100, 10000, 10)
optimize_secretary_problem_3(100, 40000, 1000)
