#' A Shiny app to plot negative binomial distributions interactively

#' App UI
#'
#' @param id An ID to namespace the module
negativeBinomialUI <- function(id) {
  fluidPage(
    titlePanel("Negative Binomial Distribution"),
    sidebarLayout(
      ## UI and Server must have a common id.
      parameterUI("nbplot"),
      mainPanel(mainPanelUI("nbplot"))
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
