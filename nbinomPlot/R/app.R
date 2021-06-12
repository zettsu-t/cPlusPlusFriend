#' A Shiny app to plot negative binomial distributions interactively

## Build this package and run
## pkgload::load_all(".")
## negativeBinomialApp()
library(shiny)

#' Launch a Shiny app
negativeBinomialApp <- function() {
  ## UI and SERVER must have a common id.
  ui <- function(input, output, session) {
    negativeBinomialUI("nbplot")
  }
  server <- function(input, output, session) {
    negativeBinomialServer("nbplot")
  }
  shinyApp(ui, server)
}

negativeBinomialApp()
