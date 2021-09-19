FROM rocker/tidyverse

## Install a language pack
RUN apt-get update
RUN apt-get install -y language-pack-ja curl
RUN update-locale LANG=ja_JP.UTF-8
RUN locale-gen ja_JP.UTF-8

## Install Japanese fonts
ARG FONT_DIR="/usr/share/fonts/truetype"
ARG FONT_ZIP=${FONT_DIR}/migu-1m-20200307.zip
## -k options is possibly required behind an HTTP proxy
RUN curl -L "https://osdn.net/frs/redir.php?m=nchc&f=mix-mplus-ipa%2F72511%2Fmigu-1m-20200307.zip" -o "${FONT_ZIP}"
RUN unzip "${FONT_ZIP}" -d "${FONT_DIR}"
RUN rm "${FONT_ZIP}"

## Install Python packages and set up for the R reticulate package
RUN apt-get install -y libpython3-dev
## -k options is possibly required behind an HTTP proxy
RUN curl -k https://bootstrap.pypa.io/get-pip.py | python3.8
RUN python3.8 -m pip install numpy

## Install a package that Stan and cmdstanr require
RUN apt-get install -y libtbb2 libxt-dev

## Write .Renviron if you run R behind an HTTP proxy

## Install R Packages without specifying their versions
RUN R -e 'install.packages("remotes")'
RUN Rscript -e 'remotes::install_version("knitr")'
RUN Rscript -e 'remotes::install_version("assertthat")'
RUN Rscript -e 'remotes::install_version("extrafont")'
RUN Rscript -e 'remotes::install_version("Rttf2pt1", version = "1.3.8")'
RUN Rscript -e 'remotes::install_version("functional")'
RUN Rscript -e 'remotes::install_version("jsonlite")'
RUN Rscript -e 'remotes::install_version("kableExtra")'
RUN Rscript -e 'remotes::install_version("lubridate")'
RUN Rscript -e 'remotes::install_version("RColorBrewer")'
RUN Rscript -e 'remotes::install_version("reticulate")'
RUN Rscript -e 'remotes::install_version("rlang")'
RUN Rscript -e 'remotes::install_version("styler")'
RUN Rscript -e 'remotes::install_version("lintr")'
RUN Rscript -e 'remotes::install_version("markdown")'
RUN Rscript -e 'remotes::install_version("xfun")'
RUN Rscript -e 'remotes::install_version("cloc", repos = c("https://cinc.rud.is", "https://cloud.r-project.org/"))'
RUN Rscript -e 'remotes::install_version("cmdstanr", repos = c("https://mc-stan.org/r-packages/", getOption("repos")))'
RUN Rscript -e 'cmdstanr::install_cmdstan(cores = 2)'
RUN Rscript -e 'remotes::install_version("posterior")'
RUN Rscript -e 'remotes::install_version("bayesplot")'

## Setup Japanese fonts for R
RUN Rscript -e 'extrafont::font_import(prompt = FALSE)'

## Change the locale from en_US.UTF-8 to ja_JP.UTF-8
ENV LANG ja_JP.UTF-8
ENV LANGUAGE ja_JP:ja
ENV LC_ALL ja_JP.UTF-8

## Create a mount point
ARG R_USERNAME=rstudio
RUN mkdir -p "/home/${R_USERNAME}/work"
