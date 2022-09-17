library(tidyverse)
library(lubridate)

df <- purrr::map(0:29, function(x) {
  end <- lubridate::ymd_hms("2022/9/17T18:00:00") + lubridate::minutes(x)
  start <- lubridate::floor_date(end, unit="day")

  timestamp <- sprintf("%02d:%02d:%02d",
    lubridate::hour(end), lubridate::minute(end), lubridate::second(end))

  fmt <- "%.16f"
  interval_sec <- sprintf(fmt,
    as.numeric(lubridate::interval(start = start, end = end), unit = "seconds"))

  duration_sec <- sprintf(fmt,
    as.numeric(lubridate::as.duration(end - start)))

  tibble::tibble(timestamp = timestamp, interval = interval_sec, duration = duration_sec)
}) %>%
  dplyr::bind_rows()

View(df)
