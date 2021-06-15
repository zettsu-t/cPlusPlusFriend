#' A Shiny app to plot negative binomial distributions interactively

#' Parameters of a negative binomial distribution UI
#'
#' @param id An ID to namespace the module
parameterUI <- function(id) {
  ## UIs must have a common id.
  shiny::sidebarPanel(
    sizeVarUI("nbplot"),
    probVarUI("nbplot"),
    muVarUI("nbplot"),
    quantileVarUI("nbplot"),
    resetButtonUI("nbplot")
  )
}

#' Parameters of a negative binomial distribution Server
#'
#' @param id An ID to namespace the module
parameterServer <- function(id) {
  shiny::moduleServer(id, function(input, output, session) {
    params <- list(
      size = shiny::reactive(input$size),
      prob = shiny::reactive(input$prob),
      mu = shiny::reactive(input$mu),
      quantile = shiny::reactive(input$quantile)
    )
    params
  })
}
