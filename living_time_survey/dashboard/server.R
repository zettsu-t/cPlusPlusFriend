library(shiny)
library(plotly)

reactive_list <- reactiveValues()
reactive_list$df_survey <- read_data_all(in_dirname = "../incoming")

render_local <- function(input, use_plotly) {
  df <- filter_by_action(
    df = reactive_list$df_survey, action = input$action, days_in_week = input$days_in_week
  )

  method <- "group"
  color_func <- pals::tol
  if (input$method == "生年") {
    method <- "cohort"
    color_func <- pals::stepped
  }

  draw_by_survey_years_for_webapp(
    df = df, action = input$action, days_in_week = input$days_in_week,
    key = method, color_func = color_func, use_plotly = use_plotly
  )
}

shinyServer(function(input, output, session) {
  observe({
    reactive_list$action_set <- unique(reactive_list$df_survey$`行動`)
    reactive_list$days_in_week_set <- unique(reactive_list$df_survey$`曜日`)
    updateSelectInput(session, "action", label = "行動", choices = reactive_list$action_set)
    updateSelectInput(session, "days_in_week", label = "曜日", choices = reactive_list$days_in_week_set)
  })
  output$main_plot <- renderPlot({
    render_local(input, use_plotly = FALSE)
  })
  output$main_plotly <- renderPlotly({
    render_local(input, use_plotly = TRUE)
  })
})
