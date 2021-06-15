#' A Shiny app to plot negative binomial distributions interactively

#' App UI
#'
#' @param id An ID to namespace the module
negativeBinomialUI <- function(id) {
  shiny::fluidPage(
    shiny::titlePanel("Negative Binomial Distribution"),
    shiny::sidebarLayout(
      ## UI and Server must have a common id.
      parameterUI("nbplot"),
      shiny::mainPanel(mainPanelUI("nbplot"))
    )
  )
}

#' App Server
#'
#' @param id An ID to namespace the module
negativeBinomialServer <- function(id) {
  params <- parameterServer("nbplot")
  mainPanelServer("nbplot", params)
}
