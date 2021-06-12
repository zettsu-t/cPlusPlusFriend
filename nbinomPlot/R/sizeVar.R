#' A Shiny app to plot negative binomial distributions interactively

#' Size parameter of a negative binomial distribution UI
#'
#' @param id An ID to namespace the module
sizeVarUI <- function(id) {
  ## Have a unique id.
  sliderInput(NS(id, "size"), "Size Paramater", min = 0.2, max = default_max_size, value = default_size)
}

#' Size parameter of a negative binomial distribution Server
#'
#' @param id An ID to namespace the module
sizeVarServer <- function(id) {
  moduleServer(id, function(input, output, session) {
    reactive(input$size)
  })
}
