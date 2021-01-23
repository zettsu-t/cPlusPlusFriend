library(tidyverse)
library(jsonlite)
library(assertthat)

params_asis <- formals(kmeans)
jsonlite::write_json(params_asis, "params_asis.json", pretty = TRUE, force=TRUE)
restored_asis <- jsonlite::fromJSON("params_asis.json")
assertthat::assert_that(!identical(params_asis, restored_asis))

convert_parameter_set <- function(params) {
  purrr::reduce(.x=names(params), .init=list(), .f=function(acc, name) {
    added <- list()
    added[[name]] <- if (is.symbol(params[[name]])) {
      NA
    } else {
      value <- params[[name]]
      if (is.call(value)) {
        eval(value)
      } else {
        value
      }
    }
    c(acc, added)
  })
}

params_eval <- convert_parameter_set(params_asis)
jsonlite::write_json(params_eval, "params_eval.json", pretty = TRUE)
restored_eval <- jsonlite::fromJSON("params_eval.json")
assertthat::assert_that(identical(params_eval, restored_eval))
