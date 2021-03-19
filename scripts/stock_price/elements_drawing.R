library(tidyverse)
library(assertthat)
library(igraph)
library(extrafont)

df_chain <- readr::read_csv("chain.csv")
assertthat::assert_that(all(head(df_chain$last, -1) == tail(df_chain$first, -1)))
assertthat::are_equal(NROW(df_chain$from), unique(NROW(df_chain$from)))
assertthat::are_equal(NROW(df_chain$Symbol), unique(NROW(df_chain$Symbol)))
paste(df_chain$Symbol, collapse = ":")
paste(df_chain$from, collapse = ":")

df_elements <- readr::read_csv("incoming/Periodic_Table_of_Elements.csv") %>%
  dplyr::distinct(AtomicNumber, .keep_all = TRUE) %>%
  dplyr::arrange(AtomicNumber) %>%
  dplyr::select("AtomicNumber", "Symbol")

nodes <- df_chain$from
edges <- as.vector(t(matrix(c(head(nodes, -1), tail(nodes, -1)), ncol = 2)))

colfunc <- colorRampPalette(c("royalblue2", "orange"))

g <- make_empty_graph() %>%
  add_vertices(max(df_elements$AtomicNumber)) %>%
  add_edges(edges)

V(g)$name <- df_elements$Symbol
E(g)$color <- colfunc(NROW(nodes) - 1)

plot(g,
  vertex.size = 1, vertex.label.family = "Segoe UI", edge.arrow.size = 0.3,
  layout = layout.circle, main = "circle"
)
