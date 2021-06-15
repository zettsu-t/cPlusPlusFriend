library(shiny)

#' @title Launch a Shiny app
#' @description This function launches an instance of the Shiny app to plot negative binomial distributions interactively
#' @export
negativeBinomialApp <- function() {
  ## UI and SERVER must have a common id.
  ui <- function(input, output, session) {
    negativeBinomialUI("nbplot")
  }
  server <- function(input, output, session) {
    negativeBinomialServer("nbplot")
  }
  shiny::shinyApp(ui, server)
}

## Build this package and run
## pkgload::load_all(".")
negativeBinomialApp()
