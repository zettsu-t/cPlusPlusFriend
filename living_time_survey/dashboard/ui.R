library(shiny)
library(shinycssloaders)
library(shinydashboard)
library(plotly)

header <- dashboardHeader(title = "国民生活時間調査")

sidebar <- dashboardSidebar(
  sidebarMenu(
    menuItem(
      "表示方法",
      menuSubItem("Plot", tabName = "TabPlot"),
      menuSubItem("Plotly", tabName = "TabPlotly")
    ),
    selectInput("action", label = "行動", choices = c("睡眠")),
    selectInput("days_in_week", label = "曜日", choices = c("平日")),
    selectInput("method", label = "年齢集団", choices = c("生年", "調査年"))
  )
)

body <- dashboardBody(
  tabItems(
    tabItem(
      "TabPlot",
      plotOutput("main_plot", width = "900px", height = "600px")
    ),
    tabItem(
      "TabPlotly",
      withSpinner(plotlyOutput("main_plotly", width = "1200px", height = "600px"))
    )
  )
)

dashboardPage(header, sidebar, body)
