library(assertthat)
library(dplyr)
library(igraph)
library(purrr)
library(readr)
library(stringr)
library(tibble)
library(extrafont)

group_lines <- function(lines, parent, node_num, prefix, pos) {
    keys <- unique(sapply(lines, function(x) stringr::str_sub(x, start=pos, end=pos)))

    ## Forward to trailing strings
    if (NROW(keys) == 1) {
        return(group_lines(lines=lines, parent=parent, node_num=node_num,
                           prefix=prefix, pos=pos+1))
    }

    sub_nodes <- purrr::reduce(.x=keys, .f=function(acc, key) {
        matched_lines <- purrr::keep(lines, function(x) {
            stringr::str_sub(x, start=pos, end=pos) == key
        })
        assertthat::assert_that(NROW(matched_lines) > 0)

        node_name <- stringr::str_sub(matched_lines[1], start=1, end=pos)
        leaf <- NROW(matched_lines) == 1
        new_nodes <- tibble(from_index=parent, to_index=acc$node_num,
                            from_name=prefix, to_name=node_name, leaf=FALSE)
        ## Increment the size of nodes
        parent <- acc$node_num
        node_num <- acc$node_num + 1

        ## Split lines recursively
        if (leaf) {
            leaf_nodes <- tibble(from_index=parent, to_index=node_num,
                                 from_name=node_name, to_name=matched_lines[1], leaf=TRUE)
            node_num <- node_num + 1
            new_nodes <- dplyr::bind_rows(new_nodes, leaf_nodes)
        } else {
            result <- group_lines(lines=matched_lines, parent=parent, node_num=node_num,
                                  prefix=node_name, pos=pos+1)
            node_num <- result$node_num
            ## NULL is acceptable here
            new_nodes <- dplyr::bind_rows(new_nodes, result$nodes)
        }

        new_nodes <- dplyr::bind_rows(acc$nodes, new_nodes)
        list(nodes=new_nodes, node_num=node_num)
    }, .init=list(nodes=NULL, node_num=node_num))

    list(nodes=sub_nodes$nodes, node_num=sub_nodes$node_num)
}

parse_lines <- function(lines) {
    line_length <- range(stringr::str_length(lines))
    min_line_length <- line_length[1]
    max_line_length <- line_length[2]

    result <- group_lines(lines=lines, parent=0, node_num=1, prefix='', pos=0)
    df <- result$nodes
    assertthat::assert_that(all(1:NROW(df) == df$to_index))
    assertthat::assert_that(sum(df$leaf) == NROW(lines))

    leaf_name_length <- stringr::str_length(as.vector(result$nodes %>% filter(leaf))$to_name)
    assertthat::assert_that(NROW(leaf_name_length) == NROW(lines))
    assertthat::assert_that(all(leaf_name_length >= min_line_length))
    assertthat::assert_that(all(leaf_name_length <= max_line_length))

    df %>% dplyr::select('from_name', 'to_name') %>%
        dplyr::rename(from=from_name, to=to_name) %>%
            dplyr::select(c('from', 'to'))
}

draw <- function(png_filename, g) {
    node_types <- pmax(1, pmin(ego_size(g, mode='out'), 3))
    colors <- c('royalblue3', 'navy', 'orange3')[node_types]
    label_font_set <- c(1, 2, 2)[node_types]
    label_size_set <- c(1.0, 1.4, 1.7)[node_types]

    png(png_filename, width=1440, height=1440)
    plot(g, layout=layout_as_tree(g, circular=TRUE), length=10,
         vertex.size=0.1, vertex.label=V(g)$name, vertex.color='white',
         vertex.label.color=colors, vertex.frame.color='white',
         vertex.label.font=label_font_set, vertex.label.cex=label_size_set,
         vertex.label.family='Migu 1M', label.font='Migu 1M',
         edge.color='gray')
    dev.off()
}

## Download from
## https://phenom.seesaa.net/article/22475232.html
## (Modern Japanese Kana-style) and convert it to UTF-8
csv_filename <- 'incoming/cards100modern.csv'
png_filename <- 'cards100.png'

lines <- (readr::read_csv(csv_filename))[[2]]
df <- parse_lines(lines)
g <- simplify(graph_from_data_frame(df, directed=TRUE, vertices=NULL))
draw(png_filename, g)
