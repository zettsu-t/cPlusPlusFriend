#' A Shiny app to plot negative binomial distributions interactively

#' Main panel UI
#'
#' @param id An ID to namespace the module
mainPanelUI <- function(id) {
  ## Have a unique id.
  plotOutput(NS(id, "plot"))
}

calculate_mu <- function(size, prob) {
  sprintf("%.3f", calculate_nbinom_mu_from_size_prob(size = size, prob = prob))
}

calculate_size <- function(prob, mu) {
  calculate_nbinom_size_from_prob_mu(prob = prob, mu = mu)
}

update_mu <- function(session, size, prob) {
  mu <- calculate_mu(size = size, prob = prob)
  updateTextInput(session, "mu", value = mu)
}

draw_nbinom <- function(lower_quantile, size, prob) {
  limit <- qnbinom(lower_quantile, size, prob)
  xs <- seq(from = 0, to = limit, by = 1)
  ys <- dnbinom(xs, size = size, prob = prob)
  df <- tibble::tibble(x = xs, density = ys)
  g <- ggplot2::ggplot(df, ggplot2::aes(x = x, y = density))
  g <- g + ggplot2::geom_line()
  g
}

#' Main panel Server
#'
#' @param id An ID to namespace the module
#' @param params Negative binomial parameters to draw
mainPanelServer <- function(id, params) {
  max_size <- reactiveVal(default_max_size)

  moduleServer(id, function(input, output, session) {
    ## Change parameters here. Not in negativeBinomialServer.
    observeEvent(input$size, {
      update_mu(session = session, size = input$size, prob = input$prob)
    })

    observeEvent(input$prob, {
      update_mu(session = session, size = input$size, prob = input$prob)
    })

    observeEvent(input$mu, {
      size <- calculate_size(prob = input$prob, mu = as.numeric(input$mu))
      max_size(max(size, max_size()))
      updateSliderInput(session, "size", max = max_size(), value = size)
    })

    observeEvent(input$reset, {
      max_size(default_max_size)
      updateSliderInput(session, "size", value = default_size, max = default_max_size)
      updateSliderInput(session, "prob", value = default_prob)
    })

    output$plot <- renderPlot({
      stopifnot(is.reactive(params$quantile))
      stopifnot(is.reactive(params$size))
      stopifnot(is.reactive(params$prob))

      lower_quantile <- as.numeric(params$quantile())
      size <- params$size()
      prob <- params$prob()
      draw_nbinom(lower_quantile = lower_quantile, size = size, prob = prob)
    })
  })
}
