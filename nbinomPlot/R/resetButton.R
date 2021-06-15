#' A Shiny app to plot negative binomial distributions interactively

#' Reset parameters of negative binomial distributions UI
#'
#' @param id An ID to namespace the module
resetButtonUI <- function(id) {
  ## Have a unique id. Do not forget NS!
  shiny::actionButton(shiny::NS(id, "reset"), "Reset parameters")
}

#' Reset parameters of negative binomial distributions Server
#'
#' @param id An ID to namespace the module
resetButtonServer <- function(id) {
  shiny::moduleServer(id, function(input, output, session) {
  })
}
