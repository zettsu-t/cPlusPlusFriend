## Use MariaDB on R
## Data source used in this script:
## https://www.city.yokohama.lg.jp/city-info/yokohamashi/tokei-chosa/portal/opendata/

library(DBI)
library(RMariaDB)
library(assertthat)

## Execute below on MySQL Client
##
## CREATE DATABASE yokohama_opendata;

conn <- dbConnect(RMariaDB::MariaDB(),
                  user = "********",
                  password = "****************",
                  host = "localhost",
                  port = 3306,
                  dbname = "yokohama_opendata")

result <- dbExecute(conn, "DROP TABLE IF EXISTS yokohama_wards;")
assertthat::assert_that(result == 0)

create_table_query <- "CREATE TABLE yokohama_wards (
  date_ymd DATE,
  id INTEGER PRIMARY KEY,
  ward VARCHAR(20) UNIQUE NOT NULL,
  area DOUBLE,
  household INTEGER,
  population INTEGER,
  male  INTEGER,
  female INTEGER,
  per_household DOUBLE,
  per_area DOUBLE,
  diff_household INTEGER,
  diff_population INTEGER
);"
result <- dbExecute(conn, create_table_query)
assertthat::assert_that(result == 0)

## Execute below on MySQL Client
##
## USE yokohama_opendata;
## LOAD DATA LOCAL INFILE '/path/to/incoming_yokohama/e1yokohama2009.csv'
##   INTO TABLE yokohama_wards FIELDS TERMINATED BY ','
##   OPTIONALLY ENCLOSED BY '"' IGNORE 1 LINES;

result <- dbExecute(conn, "DELETE FROM yokohama_wards WHERE ward NOT LIKE '%区'")
assertthat::assert_that(result == 1)

df <- dbGetQuery(conn, "SELECT * FROM yokohama_wards")
assertthat::assert_that(NROW(df) == 18)
print(df)

result <- dbDisconnect(conn)
assertthat::assert_that(result)
