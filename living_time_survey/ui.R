library(shiny)
library(shinycssloaders)
library(shinythemes)
library(plotly)

shinyUI(fluidPage(
  titlePanel("NHK放送文化研究所「国民生活時間調査」 全員平均時間量、男女年層別"),
  a(href = "https://www.nhk.or.jp/bunken/yoron-jikan/", "出典へのリンク"),
  fluidRow(
    theme = shinythemes::shinytheme("cerulean"),
    column(
      2,
      selectInput("action", label = "行動", choices = c("睡眠")),
      selectInput("days_in_week", label = "曜日", choices = c("平日")),
      selectInput("method", label = "年齢集団", choices = c("生年", "調査年")),
    ),
    column(
      10,
      tabsetPanel(
        type = "tabs",
        tabPanel("Plot",
          plotOutput("main_plot", height = "125%"),
          style = "height: 50vh"
        ),
        tabPanel("Plotly",
          withSpinner(plotlyOutput("main_plotly", height = "125%")),
          style = "height: 100vh"
        )
      )
    )
  )
))
