#' A Shiny app to plot negative binomial distributions interactively

# R CMD check --no-manual nbinomPlot
# R CMD build nbinomPlot
# R CMD INSTALL nbinomPlot_0.1.0.tar.gz
# cp nbinomPlot/launcher/app.R ~/shiny-apps/app.R
# shiny-server &

library(nbinomPlot)
negativeBinomialApp()
