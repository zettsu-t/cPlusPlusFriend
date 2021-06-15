context("test nbinomPlot screen")
library(shiny)
library(shinytest)

test_that("main_screen", {
  skip_on_os("windows")
  skip_if_not(interactive())

  ## To use expect_snapshot_file()
  testthat::local_edition(3)

  path_init_screen <- tempfile()
  path_size_screen <- tempfile()
  path_prob_screen <- tempfile()

  app <- ShinyDriver$new(negativeBinomialApp())
  app$waitForShiny()
  app$takeScreenshot(path_init_screen)
  expect_snapshot_file(path_init_screen, "tests/screenshots/init_screen.png")

  app$setValue("nbplot-size", 5.0)
  app$waitForShiny()
  app$takeScreenshot(path_size_screen)
  expect_snapshot_file(path_size_screen, "tests/screenshots/size_screen.png")

  app$setValue("nbplot-prob", 0.2)
  app$waitForShiny()
  app$takeScreenshot(path_prob_screen)
  expect_snapshot_file(path_prob_screen, "tests/screenshots/prob_screen.png")

  app$stop()
})
