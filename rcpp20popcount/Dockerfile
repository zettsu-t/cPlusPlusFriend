FROM rocker/tidyverse

RUN apt-get update && apt-get install -y libboost-dev libcurl4-openssl-dev libgit2-dev libicu-dev libssl-dev libxml2-dev cmake git-core make pandoc pandoc-citeproc qpdf zlib1g-dev
RUN apt-get install -y clang-12 lldb-12 lld-12
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-12 100
RUN update-alternatives --install /usr/bin/clang++ clang /usr/bin/clang++-12 100
RUN mkdir -p /home/rstudio/.R
RUN echo 'CXX20="clang++-12"' >> /home/rstudio/.R/Makevars
RUN chown rstudio:rstudio /home/rstudio/.R /home/rstudio/.R/Makevars

RUN R -e 'install.packages("remotes")'
RUN R -e 'install.packages(c("Rcpp", "BH", "testthat", "microbenchmark"))'

EXPOSE 8787
