library(tidyverse)
library(assertthat)
library(lpSolve)

choices <- c("08:00", "11:00", "13:00", "15:00", "19:00")
players <- c("A", "B", "C", "D", "E")

weights <- matrix(c(
  c(0, 0, 0, 0, 0), # Any
  c(1, 1, 2, 2, 0), # PM except night
  c(0, 2, 2, 1, 0), # Before or after lunch
  c(2, 2, 1, 1, 1), # AM
  c(3, 2, 1, 1, 1)  # Early
), nrow = NROW(players))

weights <- scale(weights)
# scale() generates NaNs if all elements are equal
weights[is.nan(weights)] <- 0
weights <- t(weights)

# Rows(vertical): Players
# Cols(horizontal): Choices
# Cells: Scaled preferences (scores are higher when preferable)
colnames(weights) <- choices
rownames(weights) <- players

assignment <- round(lpSolve::lp.assign(weights, direction = "max")$solution)
colnames(assignment) <- choices
rownames(assignment) <- players

assertthat::assert_that(all(assignment %in% c(0, 1)))
assertthat::assert_that(all(colSums(assignment) == 1))
assertthat::assert_that(all(rowSums(assignment) == 1))

purrr::map(seq_len(NROW(assignment)), function(player) {
  tibble(
    player = player,
    choice = colnames(assignment)[which(assignment[player, ] > 0)][1]
  )
}) %>%
  dplyr::bind_rows()
