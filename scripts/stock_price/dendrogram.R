library(tidyverse)
library(ggdendro)

## Based on
## https://cran.r-project.org/web/packages/ggdendro/vignettes/ggdendro.html

hc <- hclust(dist(USArrests), "ave")
plot(ggdendrogram(hc, rotate=TRUE, size=2))
name_set <- hc$labels

groups <- purrr::reduce2(.x=hc$merge[,1], .y=hc$merge[,2], .init=c(),
                         .f=function(acc, left, right) {
    get_name <- function(x) {
        if (x < 0) { name_set[-x] } else { acc[x] }
    }
    c(acc, paste0("(", get_name(left), " + ", get_name(right), ")"))
})

groups[1:10]
