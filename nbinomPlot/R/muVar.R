#' A Shiny app to plot negative binomial distributions interactively

#' Mu parameter of a negative binomial distribution UI
#'
#' @param id An ID to namespace the module
muVarUI <- function(id) {
  ## Have a unique id.
  shiny::textInput(shiny::NS(id, "mu"), "Mu Paramater", value = "1.0")
}

#' Mu parameter of a negative binomial distribution Server
#'
#' @param id An ID to namespace the module
muVarServer <- function(id) {
  shiny::moduleServer(id, function(input, output, session) {
    shiny::reactive(input$mu)
  })
}
