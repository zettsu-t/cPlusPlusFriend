library(graphics)
library(magick)

n_pixels <- 200

values <- runif(n_pixels * n_pixels, min = 0, max = 1)
pixels <- array(rep(values, 3), dim = c(n_pixels, n_pixels, 3))
img_fg <- magick::image_read(pixels, "rgba")

# Based on
# https://stackoverflow.com/questions/64597525/r-magick-square-crop-and-circular-mask
img_mask <- magick::image_draw(
  magick::image_blank(
    width = n_pixels, height = n_pixels, color = "none"))

graphics::symbols(x = n_pixels / 2, y = n_pixels / 2, circles = (n_pixels / 2),
                  bg = "navy", inches = FALSE, add = TRUE)
dev.off()

img_merged <- magick::image_composite(img_fg, img_mask, operator = "copyopacity")
magick::image_write(img_merged, path = "circle.png", format = "png")
