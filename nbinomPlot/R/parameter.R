#' A Shiny app to plot negative binomial distributions interactively

#' Parameters of a negative binomial distribution UI
#'
#' @param id An ID to namespace the module
parameterUI <- function(id) {
  ## UIs must have a common id.
  sidebarPanel(
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
  moduleServer(id, function(input, output, session) {
    params <- list(
      size = reactive(input$size),
      prob = reactive(input$prob),
      mu = reactive(input$mu),
      quantile = reactive(input$quantile)
    )
    params
  })
}
