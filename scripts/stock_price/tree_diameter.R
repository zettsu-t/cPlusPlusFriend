library(dplyr)
library(extrafont)
library(ggplot2)
library(grDevices)
library(igraph)
library(readr)
library(purrrlyr)

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


df <- readr::read_csv('keikyu_line.csv', col_types='cii')
stations <- df[!duplicated(df$from),]$station
df <- df %>% select(c('from', 'to'))
gr <- graph_from_data_frame(df, directed=FALSE, vertices=NULL)
gr <- simplify(gr)
V(gr)$name <- stations

## Split the graph gr into vertices along chains and junctions
section <- 1
sections <- integer()
vertices <- integer()
previous_dist <- 0
previous_junction <- FALSE

junctions <- sapply(1:NROW(stations), function(x) {
    (NROW(igraph::neighbors(gr, x)) > 2)
})

dfs_callback <- function(g, v, exta) {
    vid <- as.integer(v[['vid']] + 1)
    dist <- v[['dist']]
    is_junction <- junctions[vid]

    ## Visits or leaves a junction, starts a new backtracking
    if ((is_junction == TRUE) || (previous_junction == TRUE) || (previous_dist > dist)) {
        section <<- section + 1
    }

    sections <<- c(sections, section)
    vertices <<- c(vertices, vid)
    previous_dist <<- dist
    previous_junction <<- is_junction
    FALSE
}

igraph::dfs(gr, root=1, "all", in.callback=dfs_callback)
sections <- (tibble(section=sections, vertex=vertices) %>% dplyr::arrange(vertices))$section
gr_c <- simplify(igraph::contract(gr, sections))
V(gr_c)$name <- sapply(V(gr_c)$name, function(x) { paste(x,sep=',',collapse=',') })

png(filename='full.png', width=1024, height=512)
par(mfrow=c(1,2), family='Migu 1M')
plot(gr, layout=layout.fruchterman.reingold, main='', family='Migu 1M',
     edge.width=2, edge.color='black',
     vertex.size=1, vertex.label.cex=0.75, vertex.label.color='black')

plot(gr_c, layout=layout.fruchterman.reingold, main='', family='Migu 1M',
     edge.width=2, edge.color='black',
     vertex.size=5, vertex.label.cex=1, vertex.label.color='black')
dev.off()
