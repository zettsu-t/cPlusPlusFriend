## Reduce the number of colors in an image
##
## How to launch
##   from command line (note that --args is required):
##     Rscript reduce_image_colors.R --args -i dir/input.png -o outdir -t text-on-images
##   on Rstudio:
##     arg_set <- c('-i', 'dir/input.png', '-o', 'outdir', '-t', 'text-on-images')
##     open reduce_image_colors.R, set a working directory and the current and hit Ctrl-Alt-R
## This script reads dir/input.png, reduces its color andwrites reduced-color images in outdir/
## See reduce_image_colors() to know details of options.

library(plyr)
library(dplyr)
library(purrrlyr)
library(stringr)
library(tibble)
library(bit64)
library(car)
library(grDevices)
library(imager)
library(optparse)
library(rgl)

g_color_rgb <- 'rgb'
g_color_hsv <- 'hsv'
g_color_yuv <- 'yuv'

get_position <- function(width_height, offst_pos) {
    if (offst_pos > 0) {
        offst_pos
    } else {
        width_height + offst_pos
    }
}

add_label_to_original_image <- function(img, width, height, labels) {
    original_x <- get_position(width_height=width, offst_pos=labels$original_x)
    original_y <- get_position(width_height=height, offst_pos=labels$original_y)
    name_x <- get_position(width_height=width, offst_pos=labels$name_x)
    name_y <- get_position(width_height=height, offst_pos=labels$name_y)

    imager::draw_text(im=img, x=original_x, y=original_y, text='original',
                      color=labels$color, opacity=labels$opacity, fsize=labels$fsize) %>%
        imager::draw_text(x=name_x, y=name_y, text=labels$text,
                          color=labels$color, opacity=labels$opacity, fsize=labels$fsize)
}

read_image <- function(in_png_filename, color_model, labels) {
    ## Remove alpha channels
    start_time <- proc.time()
    img <- imager::rm.alpha(imager::load.image(in_png_filename))
    dim_img <- dim(img)
    width <- dim_img[1]
    height <- dim_img[2]
    print(paste('Read', in_png_filename))
    print(proc.time() - start_time)

    ## Value levels per pixel
    n_value_level <- 1024
    n_value_level_2 <- n_value_level * n_value_level
    n_value_level_3 <- n_value_level_2 * n_value_level

    ## Counts RGB color values
    start_time <- proc.time()
    source_img <- if (color_model == g_color_rgb) {
        img
    } else if (color_model == g_color_hsv) {
        imager::RGBtoHSV(img)
    } else if (color_model == g_color_yuv) {
        imager::RGBtoYUV(img)
    }

    img_array <- array(data=source_img, dim=c(width * height, dim_img[3] * dim_img[4]))
    df <- tibble::as_tibble(img_array) %>% dplyr::rename(c1=V1, c2=V2, c3=V3)
    df$color <- as.integer64(df$c1 * n_value_level_3) + as.integer64(df$c2 * n_value_level_2) + as.integer64(df$c3 * n_value_level)
    n_colors <- NROW(unique(df$color))
    print(paste(n_colors, 'colors found'))
    print(proc.time() - start_time)

    img_label <- NA
    if (!is.null(labels)) {
        print(paste('Draw label', in_png_filename))
        img_label <- add_label_to_original_image(img=img, width=width, height=height, labels=labels)
        print(proc.time() - start_time)
    }

    list(df=df %>% select(-c('color')), img=img, img_label=img_label, width=width, height=height, n_colors=n_colors)
}

add_label_to_new_image <- function(img, width, height, labels, text) {
    colors_x <- get_position(width_height=width, offst_pos=labels$colors_x)
    colors_y <- get_position(width_height=height, offst_pos=labels$colors_y)
    imager::draw_text(im=img, x=colors_x, y=colors_y, text=text,
                      color=labels$color, opacity=labels$opacity, fsize=labels$fsize)
}

cluster_colors <- function(df, n_colors) {
    start_time <- proc.time()

    ## Reduces colors by the k-means algorithm
    ## Try other clustering and dimension-reducing methods!
    fit <- kmeans(x=df, centers=n_colors, iter.max=100000)
    print('k-means')
    print(proc.time() - start_time)

    ## Maps pixels to cluster numbers
    start_time <- proc.time()
    cluster <- as_tibble(fit$cluster)
    color_map <- as_tibble(fit$centers)
    color_map$value <- 1:NROW(color_map)

    ## Faster than by_row()
    new_image_df <- inner_join(cluster, color_map, by='value')
    print(paste0('Make a ', n_colors, '-colors map'))
    print(proc.time() - start_time)

    list(color_map=color_map, new_image_df=new_image_df)
}

reduce_colors <- function(img_data, labels, n_colors, color_model, show_images, out_png_filenames) {
    img_reduced <- cluster_colors(df=img_data$df, n_colors=n_colors)

    ## Saves the new RGB image
    start_time <- proc.time()
    new_img <- imager::as.cimg(obj=c(img_reduced$new_image_df$c1, img_reduced$new_image_df$c2, img_reduced$new_image_df$c3),
                               x=img_data$width, y=img_data$height, cc=3)
    new_img <- if (color_model == g_color_rgb) {
        new_img
    } else if (color_model == g_color_hsv) {
        imager::HSVtoRGB(new_img)
    } else if (color_model == g_color_yuv) {
        imager::YUVtoRGB(new_img)
    }
    plot(new_img)

    out_filename <- out_png_filenames$post
    imager::save.image(im=new_img, file=out_filename)

    ## Saves the new image with a label
    text <- paste0(stringr::str_to_upper(color_model), ' ', n_colors, ' colors')
    new_img_label <- add_label_to_new_image(img=new_img, width=img_data$width, height=img_data$height, labels=labels, text=text)
    out_filename <- out_png_filenames$post_label
    imager::save.image(im=new_img_label, file=out_filename)

    ## Combines the original and new images and saves it
    joint_horizontal_img <- imager::imappend(imlist=list(img_data$img, new_img), axis='x')
    out_filename <- out_png_filenames$joint_horizontal
    imager::save.image(im=joint_horizontal_img, file=out_filename)

    joint_vertical_img <- imager::imappend(imlist=list(img_data$img, new_img), axis='y')
    out_filename <- out_png_filenames$joint_vertical
    imager::save.image(im=joint_vertical_img, file=out_filename)

    joint_horizontal_img_label <- imager::imappend(imlist=list(img_data$img_label, new_img_label), axis='x')
    out_filename <- out_png_filenames$joint_horizontal_label
    imager::save.image(im=joint_horizontal_img_label, file=out_filename)

    joint_vertical_img_label <- imager::imappend(imlist=list(img_data$img_label, new_img_label), axis='y')
    out_filename <- out_png_filenames$joint_vertical_label
    imager::save.image(im=joint_vertical_img_label, file=out_filename)

    print(paste0('Joint images and save PNG file', color_model))
    print(proc.time() - start_time)

    if (show_images == TRUE) {
        plot(new_img_label)

        surface_colors <- c()
        labels_3d <- list(x='', y='', z='')
        if (color_model == g_color_rgb) {
            surface_colors <- grDevices::rgb(img_reduced$color_map$c1, img_reduced$color_map$c2, img_reduced$color_map$c3)
            labels_3d <- list(x='Red', y='Green', z='Blue')
        } else if (color_model == g_color_hsv) {
            surface_colors <- grDevices::hsv(img_reduced$color_map$c1 / 360.0, img_reduced$color_map$c2, img_reduced$color_map$c3)
            labels_3d <- list(x='Hue', y='Saturation', z='Value')
        } else if (color_model == g_color_yuv) {
            surface_colors <- grDevices::rgb(pmax(0.0, pmin(1.0, (img_reduced$color_map$c1 + 1.402 * img_reduced$color_map$c3))),
                                             pmax(0.0, pmin(1.0, (img_reduced$color_map$c1 - 0.244 * img_reduced$color_map$c2 - 0.714 * img_reduced$color_map$c3))),
                                             pmax(0.0, pmin(1.0, (img_reduced$color_map$c1 + 1.772 * img_reduced$color_map$c2))))
            labels_3d <- list(x='Y', y='U', z='V')
        }

        rgl::par3d(c(10, 10, 240, 240))
        car::scatter3d(x=img_reduced$color_map$c1, y=img_reduced$color_map$c2, z=img_reduced$color_map$c3,
                       xlab=labels_3d$x, ylab=labels_3d$y, zlab=labels_3d$z,
                       axis.col=rep('black', 3), surface.col=surface_colors,
                       surface=FALSE, sphere.size=2, groups=factor(1:NROW(img_reduced$color_map)),
                       ellipsoid.alpha=0.03)

        invisible(readline(prompt='Press [enter] to continue'))
    }

    list(new_img=new_img, new_img_label=new_img_label,
         joint_horizontal_img=joint_horizontal_img, joint_horizontal_img_label=joint_horizontal_img_label,
         joint_vertical_img=joint_vertical_img, joint_vertical_img_label=joint_vertical_img_label)
}

reduce_n_color_set <- function(original_filename, labels, n_color_set, color_model, show_images, out_dirname, no_reduced_images) {
    img_data <- read_image(in_png_filename=original_filename, color_model=color_model, labels=labels)

    images <- lapply(n_color_set, function(n_colors) {
        file_basename <- paste0('color_', n_colors, '_', basename(original_filename)) %>%
            stringr::str_replace_all('(.*?)\\.[^.]*$', paste0('\\1', '_', color_model))

        joint_horizontal_prefix <- 'joint_horizontal'
        joint_vertical_prefix <- 'joint_vertical'
        no_label_suffix <- '.png'
        label_suffix <- '_label.png'
        out_png_filenames <- list(post=file.path(out_dirname, paste0(file_basename, no_label_suffix)),
                                  post_label=file.path(out_dirname, paste0(file_basename, label_suffix)),
                                  joint_horizontal=file.path(out_dirname, paste0(joint_horizontal_prefix, file_basename, no_label_suffix)),
                                  joint_horizontal_label=file.path(out_dirname, paste0(joint_horizontal_prefix, file_basename, label_suffix)),
                                  joint_vertical=file.path(out_dirname, paste0(joint_vertical_prefix, file_basename, no_label_suffix)),
                                  joint_vertical_label=file.path(out_dirname, paste0(joint_vertical_prefix, file_basename, label_suffix)))

        images <- reduce_colors(img_data=img_data, labels=labels, n_colors=n_colors, color_model=color_model,
                                show_images=show_images, out_png_filenames=out_png_filenames)

        ## Checks whether this script reduced the number of colors in the input image
        verify_data <- read_image(in_png_filename=out_png_filenames$post, color_model=g_color_rgb, labels=NULL)
        if (g_color_rgb == color_model) {
            assertthat::assert_that(n_colors == verify_data$n_colors)
        }

        if (no_reduced_images == TRUE) {
            NA
        } else {
            images
        }
    })

    if (no_reduced_images == TRUE) {
        list(img_data=img_data, images=NA)
    } else {
        list(img_data=img_data, images=images)
    }
}

reduce_image_colors <- function(args, n_color_set) {
    parser <- OptionParser()
    parser <- optparse::add_option(parser, c('-i', '--input'), type='character', action='store',
                                   dest='input_filename', default='incoming/input.png', help='Input original image file')

    parser <- optparse::add_option(parser, c('-o', '--outdir'), type='character', action='store',
                                   dest='out_dirname', default='out', help='Output directory')

    ## The text is drawn on copies of the input images to show its credit or byline.
    parser <- optparse::add_option(parser, c('-t', '--text'), type='character', action='store',
                                   dest='text', default='by whom', help='Text on the original image')

    parser <- optparse::add_option(parser, c('-x', '--no_reduced_images'), action='store_true',
                                   dest='no_reduced_images', default=FALSE, help='Do not keep generated images on memory')

    parser <- optparse::add_option(parser, c('-s', '--show_images'), action='store_true',
                                   dest='show_images', default=FALSE, help='Show interactive 2D and 3D images')

    parser <- optparse::add_option(parser, c('-m', '--color_model'), type='character', action='store',
                                   dest='color_model', default=g_color_rgb, help='Color model')

    opts <- optparse::parse_args(parser, args=args)

    original_filename <- opts$input_filename
    out_dirname <- opts$out_dirname
    text <- opts$text

    if ((stringr::str_length(out_dirname) > 0) && (dir.exists(out_dirname) == FALSE)) {
        if (dir.create(out_dirname) == FALSE) {
            stop(paste0('Failed in creating ', out_dirname, '/'))
        }
    }

    ## Do not write to /
    if (stringr::str_length(out_dirname) == 0) {
        out_dirname <- '.'
    }

    ## Adjust the width of characters
    name_x <- -(12 + stringr::str_length(text) * 9)
    labels <- list(original_x=-80, original_y=2,
                   name_x=name_x, name_y=-20, text=text,
                   colors_x=2, colors_y=2, color='white', opacity=1, fsize=6)

    result <- reduce_n_color_set(original_filename=original_filename, labels=labels,
                                 n_color_set=n_color_set, color_model=opts$color_model, show_images=opts$show_images,
                                 out_dirname=out_dirname, no_reduced_images=opts$no_reduced_images)
    list(original=result$img_data, images=result$images, color_model=opts$color_model, out_dirname=out_dirname)
}

get_args <- function() {
    ## Set args in Rstudio
    if (exists('arg_set') == FALSE) {
        arg_set <- c()
    }

    command_args <- commandArgs(trailingOnly=FALSE)
    if ((NROW(command_args) > 0) && (command_args[1] != 'RStudio')) {
        ## Exclude --args
        arg_set <- tail(commandArgs(trailingOnly=TRUE), -1)
    }

    print("This script takes args")
    print(arg_set)
    arg_set
}

write_8_level_images <- function() {
##  n_color_set <- c(2, 4, 8, 16, 32, 64, 128, 256)
    n_color_set <- c(256)
    result <- reduce_image_colors(args=get_args(), n_color_set=n_color_set)
    if (is.na(result$images)) {
        return(NA)
    }

    color_model <- result$color_model
    out_dirname <- result$out_dirname

    ## Do not write to /
    if (stringr::str_length(out_dirname) == 0) {
        out_dirname <- '.'
    }

    img_upper_1 <- imager::imappend(imlist=list(result$images[1][[1]]$new_img_label, result$images[2][[1]]$new_img_label), axis='x')
    img_lower_1 <- imager::imappend(imlist=list(result$images[3][[1]]$new_img_label, result$images[4][[1]]$new_img_label), axis='x')
    img_upper_2 <- imager::imappend(imlist=list(result$images[5][[1]]$new_img_label, result$images[6][[1]]$new_img_label), axis='x')
    img_lower_2 <- imager::imappend(imlist=list(result$images[7][[1]]$new_img_label, result$images[8][[1]]$new_img_label), axis='x')

    img_wide_1 <- imager::imappend(imlist=list(img_upper_1, img_lower_1), axis='x')
    img_square_1 <- imager::imappend(imlist=list(img_upper_1, img_lower_1), axis='y')
    img_wide_2 <- imager::imappend(imlist=list(img_upper_2, img_lower_2), axis='x')
    img_square_2 <- imager::imappend(imlist=list(img_upper_2, img_lower_2), axis='y')
    img_wide_12 <- imager::imappend(imlist=list(img_wide_1, img_wide_2), axis='x')
    img_all <- imager::imappend(imlist=list(img_wide_12, result$original$img_label), axis='x')

    make_png_filename <- function(filename) {
        paste0(filename, color_model, '.png')
    }

    imager::save.image(im=img_wide_1, file=file.path(out_dirname, make_png_filename('img_wide_1')))
    imager::save.image(im=img_wide_2, file=file.path(out_dirname, make_png_filename('img_wide_2')))
    imager::save.image(im=img_square_1, file=file.path(out_dirname, make_png_filename('img_square_1')))
    imager::save.image(im=img_square_2, file=file.path(out_dirname, make_png_filename('img_square_2')))
    imager::save.image(im=img_wide_12, file=file.path(out_dirname, make_png_filename('img_wide_12')))
    imager::save.image(im=img_all, file=file.path(out_dirname, make_png_filename('img_all')))
    result
}

result <- write_8_level_images()
