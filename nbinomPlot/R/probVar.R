#' A Shiny app to plot negative binomial distributions interactively

#' Prob parameter of a negative binomial distribution UI
#'
#' @param id An ID to namespace the module
probVarUI <- function(id) {
  ## Have a unique id.
  shiny::sliderInput(shiny::NS(id, "prob"), "Prob Paramater", min = 0.01, max = 1.0, value = default_prob)
}

#' Prob parameter of a negative binomial distribution Server
#'
#' @param id An ID to namespace the module
probVarServer <- function(id) {
  shiny::moduleServer(id, function(input, output, session) {
    shiny::reactive(input$prob)
  })
}
