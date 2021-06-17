#' A Shiny app to plot negative binomial distributions interactively

#' Update the Mu parameter of negative binomial distributions UI
#'
#' @param id An ID to namespace the module
updateMuButtonUI <- function(id) {
  ## Have a unique id. Do not forget NS!
  shiny::actionButton(shiny::NS(id, "update_mu"), "Update Mu")
}

#' Update the Mu parameter of negative binomial distributions Server
#'
#' @param id An ID to namespace the module
updateMuButtonServer <- function(id) {
  shiny::moduleServer(id, function(input, output, session) {
  })
}
