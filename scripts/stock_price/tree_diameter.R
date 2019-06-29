library(dplyr)
library(ggplot2)
library(grDevices)
library(igraph)

to_leaves <- function(x, max_weight) {
    weight=max(0, max_weight - log2(x)) + 1
    data.frame(from=c(x,x),
               to=c(as.integer(x*2),as.integer(x*2+1)),
               weight=c(weight, weight))
}

make_tree <- function(n_level, max_weight) {
    n_vertex <- 2**(n_level-2)
    n_vertex <- n_vertex * 2 + n_vertex
    df <- lapply(1:n_vertex, function(x) {to_leaves(x, max_weight)} ) %>% dplyr::bind_rows()
    gr <- graph_from_data_frame(df, directed=FALSE, vertices = NULL)
    print(igraph::diameter(gr))
    print(igraph::get_diameter(gr))

    font_size_step <- 0.4
    font_sizes <- as.integer(df[!duplicated(df$to),]$weight) * font_size_step + 1.0
    font_sizes <- c(max(font_sizes) + font_size_step, font_sizes)

    weights <- as.integer(df$weight)
    weights <- weights - min(weights) + 1
    colfunc <- colorRampPalette(c('forestgreen', 'olivedrab'))(max(weights))
    E(gr)$color <- colfunc[weights]
    print(igraph::diameter(gr))

    out_chart_filename <- paste0('tree_graph_', max_weight, '.png', sep='')
    png(filename=out_chart_filename, width=600, height=600)
    plot(gr, layout=layout.fruchterman.reingold, main='',
         edge.width=E(gr)$weight * 2, edge.color=E(gr)$color,
         vertex.size=1, vertex.label.cex=font_sizes, vertex.label.color='black')
    dev.off()
}

n_level <- 5
make_tree(n_level, 1)
make_tree(n_level, n_level)
