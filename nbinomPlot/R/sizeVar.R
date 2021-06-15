#' A Shiny app to plot negative binomial distributions interactively

#' Size parameter of a negative binomial distribution UI
#'
#' @param id An ID to namespace the module
sizeVarUI <- function(id) {
  ## Have a unique id.
  shiny::sliderInput(shiny::NS(id, "size"), "Size Paramater",
                     min = 0.2, max = default_max_size, value = default_size)
}

#' Size parameter of a negative binomial distribution Server
#'
#' @param id An ID to namespace the module
sizeVarServer <- function(id) {
  shiny::moduleServer(id, function(input, output, session) {
    shiny::reactive(input$size)
  })
}
