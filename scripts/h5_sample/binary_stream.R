library(assertthat)
library(magick)
library(rhdf5)

create_h5_file <- function(h5_filename, image_filename) {
  if (file.exists(h5_filename)) {
    file.remove(h5_filename)
  }

  image_data <- readBin(con = image_filename, what = integer(),
                        n = file.info(image_filename)$size,
                        size = 1, signed = FALSE)

  h5createFile(h5_filename)
  h5createGroup(h5_filename, "top")
  h5write(as.array(image_data), h5_filename, "top/image")
  h5closeAll()
}

read_h5_file <- function(h5_filename, node, image_filename) {
  if (!file.exists(h5_filename)) {
    return()
  }

  image <- h5read(h5_filename, node)
  png_filename <- tempfile(fileext=".png")
  writeBin(as.raw(image), png_filename)
  h5closeAll()

  actual_image <- magick::image_read(png_filename)
  expected_image <- magick::image_read(image_filename)
  plot(actual_image)

  assertthat::assert_that(identical(as.raster(actual_image),
                                    as.raster(expected_image)))
}

create_h5_file(h5_filename = "r.h5", image_filename = "blue.png")
read_h5_file(h5_filename = "r.h5", node = "top/image", image_filename = "blue.png")
read_h5_file(h5_filename = "python.h5", node = "root/image", image_filename = "orange.png")
