library(shiny)
library(plotly)

shinyUI(fluidPage(
  titlePanel("NHK放送文化研究所「国民生活時間調査」 全員平均時間量、男女年層別"),
  a(href = "https://www.nhk.or.jp/bunken/yoron-jikan/", "出典へのリンク"),
  fluidRow(
    column(
      2,
      selectInput("action", label = "行動", choices = g_action_set),
      selectInput("days_in_week", label = "曜日", choices = g_days_in_week_set),
      selectInput("method", label = "年齢集団", choices = c("生年", "調査年")),
      selectInput("render", label = "描画方法", choices = c("plot", "plotly"))
    ),
    column(
      10,
      conditionalPanel(
        condition = "input.render == 'plotly'",
        plotlyOutput("main_plotly", height = "125%"), style = "height: 60vh"
      ),
      conditionalPanel(
        condition = "input.render != 'plotly'",
        plotOutput("main_plot", height = "125%"), style = "height: 50vh"
      )
    )
  )
))
