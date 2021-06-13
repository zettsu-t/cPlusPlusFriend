#' A Shiny app to plot negative binomial distributions interactively

#' Quantile to display UI
#'
#' @param id An ID to namespace the module
quantileVarUI <- function(id) {
  ## Have a unique id.
  selectInput(NS(id, "quantile"), "Coverage of quantile", choices = c(0.99, 0.999, 0.9999, 0.99999))
}

#' Quantile to display Server
#'
#' @param id An ID to namespace the module
quantileVarServer <- function(id) {
  moduleServer(id, function(input, output, session) {
    reactive(input$quantile)
  })
}
