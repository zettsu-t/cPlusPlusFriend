library(plyr)
library(dplyr)
library(purrr)
library(purrrlyr)

## Short scripts to tweet
tibble(a=c(7,14,23),b=c(2,3,4))%>%by_row(function(x){tibble(q=x$a%/%x$b,r=x$a%%x$b)},.collate="rows")

s<-c()
f<-function(x,y) {
 r<-!(y%in%s)
 s<<-unique(append(s,y))
 append(x,r)
}
last(purrr::accumulate(.x=c(1,2,3,2,4), .f=f, .init=c()))

s<-c()
f<-function(x) {
 r<-!(x%in%s)
 s<<-unique(append(s,x))
 r
}
purrr::map_lgl(.x=c(1,2,3,2,4), .f=f)

## Scripts with proper names
mydiv <- function(row) {
    tibble(quotient=row$dividend %/% row$divisor,
           remainder=row$dividend %% row$divisor)
}

df <- tibble(dividend=c(7,14,23), divisor=c(2,3,4))
df %>% by_row(mydiv, .collate="rows")

seen_member <- c()
member <- c(1, 2, 3, 2, 4, 5, 3, 6)
finder <- function(x, y) {
    fst <- !(y %in% seen_member)
    seen_member <<- unique(append(seen_member, y))
    append(x, fst)
}
tail(purrr::accumulate(.x=member, .f=finder, .init=c()), n=1)

seen_member <- c()
member <- c(1, 2, 3, 2, 4, 5, 3, 6)
finder <- function(x) {
    fst <- !(x %in% seen_member)
    seen_member <<- unique(append(seen_member, x))
    fst
}
purrr::map_lgl(.x=member, .f=finder)
