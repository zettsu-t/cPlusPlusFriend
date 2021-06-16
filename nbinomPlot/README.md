nbinomPlot
==============

This Shiny app plots negative binomial distributions interactively on RStudio or Web browsers.

Run this package locally
------------

When you run this package locally, it is not necessary to install it.

1. Open __nbinomPlot/nbinomPlot.Rproj__ on RStudio.
1. Open __R/app.R__ on the RStudio Files tab.
1. Execute `devtools::load_all()` on the RStudio console.
1. Push the _Run App_ button on the __app.R__ tab.

Install this package
------------

Run these commands below. Make sure _devtools_ package is installed.

```bash
R CMD build nbinomPlot --no-manual --no-build-vignettes
R CMD INSTALL nbinomPlot_0.1.0.tar.gz
```

Run this package on Shiny Server
------------

To deploy this package to your Shiny Server, install this package and copy __nbinomPlot/launcher/app.R__ to a public directory of your Shiny Server which is possibly linked from __/srv/shiny-server__ .

```r
cp nbinomPlot/launcher/app.R /path/to/public/app-dir/
```

Access a URL like _http://shiny.example.com/app-dir_ from a Web browser and you can view a negative binomial distribution plot.

Test and check this package
------------

You can test source code in this package on R.

1. Open __nbinomPlot/nbinomPlot.Rproj__ on RStudio or launch the R command on a terminal.
1. Execute . `devtools::test()` on the console.

Note that __nbinomPlot/tests/testthat/\_snaps/screen/tests/screenshots/__ is not created automatically and you have to create the output directory if you save snapshots in tests of this package.

You can test this package in one command.

```bash
R CMD check nbinomPlot --no-manual --no-build-vignettes
```
