library(tidyverse)
library(assertthat)
library(cluster)
library(e1071)
library(extrafont)
library(factoextra)
library(fclust)
library(ppclust)

## Based on
## https://blog.8tak4.com/post/148145887635/r-cluster-analyses
## https://rpubs.com/rahulSaha/Fuzzy-CMeansClustering

base_to_height <- sqrt(3.0) * 0.5 ## 正三角形の高さ/底辺
long_to_short <- 1.0 / sqrt(3.0) ## 二等分した正三角形の短辺/長辺
font_name <- "Segoe UI"

## 元のカテゴリと、クラスタリングによる分類番号を合わせる
## クラスタリングのクラスタが何番になるかは不定なので
## mat_clustersは3クラスタそれぞれの寄与率
## categoriesは1..3クラスタの名前
match_clusters <- function(mat_clusters, categories) {
  ## カテゴリの名前に番号をつける
  ys_factors <- factor(categories)
  name_set <- levels(ys_factors)
  levels <- as.numeric(ys_factors)

  ## 各行がクラスタ番号、各列が元のカテゴリ、セルは両者が一致する標本数を返す
  map_cluster_to_level <- function(mat) {
    # ハードクラスタリング
    clusters <- max.col(mat)
    cluster_set <- sort(unique(clusters))
    assertthat::assert_that(assertthat::are_equal(NROW(name_set), NROW(unique(cluster_set))))

    purrr::reduce(seq_len(NROW(name_set)),
      .init = c(), function(acc, level) {
        ## 縦に並べたものを横に並べる
        c(acc, purrr::map_dbl(cluster_set, function(cluster) {
          sum((levels == level) * (clusters == cluster))
        }))
      }
    ) %>%
      matrix(nrow = NROW(cluster_set), ncol = NROW(name_set), byrow = FALSE)
  }

  ## クラスタ番号を並べ替える前
  base_cluster_to_level <- map_cluster_to_level(mat_clusters)
  ## 外れが少ないクラスタから順に取ってくる
  clusters_from <- order(
    purrr::map_dbl(
      seq_len(NROW(base_cluster_to_level)),
      ~ max(base_cluster_to_level[.x, ])
    ), decreasing = TRUE
  )
  sorted_mat_members <- mat_clusters
  sorted_mat_members[, seq_len(NCOL(sorted_mat_members))] <- sorted_mat_members[, clusters_from]

  ## クラスタ番号を並べ替えた後
  new_cluster_to_level <- map_cluster_to_level(sorted_mat_members)
  ## それぞれのクラスタに、最も当てはまりのよいカテゴリを割り当てる
  cluster_names <- name_set[max.col(new_cluster_to_level)]
  assertthat::assert_that(assertthat::are_equal(NROW(cluster_names), NROW(unique(cluster_names))))
  list(cluster_names = cluster_names, mat_members = sorted_mat_members)
}

## 3クラスタの帰属率を、三角形の辺からの距離で表現するときの座標を求める
## matは各列が標本、各行が3クラスタそれぞれの寄与率(足すと和が1)
make_xy_pos <- function(mat) {
  ## クラスタ1を上の頂点、クラスタ2を右の頂点にする
  scale_1 <- long_to_short
  scale_2 <- base_to_height
  ys <- mat[, 1] * scale_2
  zs <- mat[, 2] * scale_2
  xs <- (ys + zs * 0.5) * scale_1 + zs * scale_2
  tibble::tibble(x = xs, y = ys)
}

## 3クラスタの帰属率を、三角形の辺からの高さで表現する
## mat_clustersは3クラスタそれぞれの寄与率
## categoriesは1..3クラスタの名前
draw_3_clusters <- function(mat_clusters, categories, title, out_filename) {
  center_x <- 0.5
  center_y <- 0.5 * long_to_short
  half_y <- base_to_height * 0.5

  ## 三角形とその内部の辺
  df_segment <- tibble::tibble(
    x = c(0, 0.5, 1.0, rep(center_x, 3)), y = c(0, base_to_height, 0, rep(center_y, 3)),
    xend = c(0.5, 1.0, 0, 0.5, 0.25, 0.75), yend = c(base_to_height, 0, 0, 0, half_y, half_y)
  )

  ## それぞれの頂点がどのカテゴリかを確定する
  normalized_clusters <- mat_clusters * (1.0 / rowSums(mat_clusters))
  sorted_clusters <- match_clusters(mat_clusters = normalized_clusters, categories = categories)
  cluster_names <- sorted_clusters$cluster_names
  mat_members <- sorted_clusters$mat_members

  ## 頂点を確定してから標本の座標を決める
  df_xy_pos <- make_xy_pos(mat = mat_members) %>%
    dplyr::mutate(category = factor(categories))

  ## 頂点の位置と名前
  df_label <- tibble(x = c(0.5, 0.94, 0.05), y = c(base_to_height + 0.05, -0.05, -0.05), label = cluster_names)

  g <- ggplot()
  g <- g + geom_point(aes(x = x, y = y, color = category, shape = category), data = df_xy_pos, size = 5, alpha = 0.8)
  g <- g + scale_color_manual(values = c("gray50", "royalblue", "orange"))
  g <- g + scale_shape_manual(values = 3:5)
  g <- g + geom_segment(aes(x = x, y = y, xend = xend, yend = yend), data = df_segment)
  g <- g + geom_text(aes(x = x, y = y, label = label), data = df_label, size = 5, family = font_name)
  if (!is.na(title)) {
    g <- g + ggtitle(title)
  }
  g <- g + theme_bw()
  g <- g + theme(
    aspect.ratio = base_to_height, legend.position = "none",
    panel.border = element_blank(), axis.ticks = element_blank(),
    panel.grid.major = element_blank(), panel.grid.minor = element_blank(),
    axis.text = element_blank(), axis.title = element_blank()
  )

  ggsave(out_filename, g, dpi = 75)
  list(g = g, mat_members = mat_members, sorted_clusters = sorted_clusters)
}

ppclust_fcm <- function(x_set, y_set, title, out_filename) {
  model <- ppclust::fcm(x_set, centers = 3)
  result <- draw_3_clusters(
    mat_clusters = model$u, categories = y_set,
    title = title, out_filename = out_filename
  )
  list(model = model, result = result)
}

e1071_cmeans <- function(x_set, y_set, title, out_filename) {
  model <- e1071::cmeans(x_set, centers = 3)
  result <- draw_3_clusters(
    mat_clusters = model$membership, categories = y_set,
    title = title, out_filename = out_filename
  )
  list(model = model, result = result)
}

fclust_Fclust <- function(x_set, y_set, title, out_filename) {
  model <- fclust::Fclust(x_set, k = 3)
  result <- draw_3_clusters(
    mat_clusters = model$U, categories = y_set,
    title = title, out_filename = out_filename
  )
  list(model = model, result = result)
}

execute_all <- function() {
  data(iris)
  x_set <- iris[, -5]
  y_set <- iris[, 5]

  result <- ppclust_fcm(
    x_set = x_set, y_set = y_set, title = NA,
    out_filename = "ppclust_no_title.png"
  )

  result_fcm <- ppclust_fcm(
    x_set = x_set, y_set = y_set, title = "ppclust::fcm",
    out_filename = "ppclust.png"
  )

  result_cmeans <- e1071_cmeans(
    x_set = x_set, y_set = y_set, title = "e1071::cmeans",
    out_filename = "cmeans.png"
  )

  result_fclust <- fclust_Fclust(
    x_set = x_set, y_set = y_set, title = "fclust::Fclust",
    out_filename = "fclust.png"
  )
}

execute_all()
