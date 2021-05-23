library(shiny)
library(plotly)

render_local <- function(input) {
  df <- filter_by_action(df = g_df_survey, action = input$action, days_in_week = input$days_in_week)

  method <- "group"
  color_func <- pals::tol
  if (input$method == "生年") {
    method <- "cohort"
    color_func <- pals::stepped
  }
  use_plotly <- (input$render == "plotly")

  draw_by_survey_years_for_webapp(
    df = df, action = input$action, days_in_week = input$days_in_week,
    key = method, color_func = color_func, use_plotly = use_plotly
  )
}

shinyServer(function(input, output, session) {
  output$main_plot <- renderPlot({
    render_local(input)
  })
  output$main_plotly <- renderPlotly({
    render_local(input)
  })
})
