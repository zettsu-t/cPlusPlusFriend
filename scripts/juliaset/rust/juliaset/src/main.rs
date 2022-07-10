#[cfg_attr(test, macro_use)]
extern crate assert_float_eq;

use csv::WriterBuilder;
use image::RgbImage;
use ndarray::prelude::*;
use ndarray_csv::Array2Writer;
use std::convert::TryFrom;
use std::fs::File;

/// A count that represents how many times a point is transformed
type Count = i32;

/// A coordinate in screens
type Coordinate = f32;

/// A coordinate set in screens
type Point = num::complex::Complex<Coordinate>;

/// A set of coordinates
type CoordinateSet = ndarray::Array1<Coordinate>;

/// RGB colors at pixels in a screen
type Bitmap = ndarray::Array3<u8>;

/// All counts in a screen
type CountSet = ndarray::Array2<Count>;

/// Returns a transformed point under the rule of Julia sets
///
/// # Arguments
///
/// * `from` An original point
/// * `offset` An offset to be added
fn transform_point(from: Point, offset: Point) -> Point {
    from * from + offset
}

/// Returns how many times a point is transformed
///
/// # Arguments
///
/// * `point_x` The x coordinate of a point
/// * `point_y` The y coordinate of a point
/// * `offset` An offset to be added
/// * `max_iter` The maximum number of iterations
/// * `eps` Tolerance to check if transformations are converged
fn converge_point(
    point_x: Coordinate,
    point_y: Coordinate,
    offset: Point,
    max_iter: Count,
    eps: Coordinate,
) -> Count {
    let mut z = Point::new(point_x, point_y);
    let mut count: Count = 0;
    let mut previous_modulus: Coordinate = 10.0;

    for _ in 0..max_iter {
        z = transform_point(z, offset);
        let z_modulus = z.norm_sqr();
        if z_modulus > 4.0 {
            break;
        }
        if (previous_modulus - z_modulus).abs() < eps {
            break;
        }
        count += 1;
        previous_modulus = z_modulus;
    }

    count
}

/// Returns how many times each point in a screen is transformed
///
/// # Arguments
///
/// * `xs` X coordinates of points in a screen
/// * `ys` Y coordinates of points in a screen
/// * `offset` An offset to be added
/// * `max_iter` The maximum number of iterations
/// * `eps` Tolerance to check if transformations are converged
fn converge_point_set(
    xs: CoordinateSet,
    ys: CoordinateSet,
    offset: Point,
    max_iter: Count,
    eps: Coordinate,
) -> CountSet {
    let x_elements = xs.shape()[0];
    let y_elements = ys.shape()[0];
    let shape = [y_elements, x_elements];
    let mut mat_counts = CountSet::zeros(shape);

    for (y_index, point_y) in ys.iter().enumerate() {
        for (x_index, point_x) in xs.iter().enumerate() {
            mat_counts[[y_index, x_index]] =
                converge_point(*point_x, *point_y, offset, max_iter, eps);
        }
    }

    mat_counts
}

/// Returns pixels on a axis in a screen
///
/// # Arguments
///
/// * `half_length` Maximum x and y coordinates relative to (0,0)
/// * `n_pixels` Numbers of pixels in X and Y axes
fn map_coordinates(half_length: Coordinate, n_pixels: isize) -> CoordinateSet {
    let n = n_pixels as Coordinate;
    (0..n_pixels)
        .map(|i| ((i as Coordinate) * 2.0 * half_length / n) - half_length)
        .collect()
}

/// Returns how many times each point in a screen is transformed
///
/// # Arguments
///
/// * `x_offset` An x offset to be added
/// * `y_offset` A y offset to be added
/// * `max_iter` The maximum number of iterations
/// * `n_pixels` Numbers of pixels in X and Y axes
fn scan_points(
    x_offset: Coordinate,
    y_offset: Coordinate,
    max_iter: Count,
    n_pixels: isize,
) -> CountSet {
    let half_length = (2.0 as Coordinate).sqrt() + 0.1;
    let xs = map_coordinates(half_length, n_pixels);
    let ys = map_coordinates(half_length, n_pixels);
    let offset = Point::new(x_offset, y_offset);
    let eps = 1e-5;
    converge_point_set(xs, ys, offset, max_iter, eps)
}

/// Draws a PNG image from an input screen
///
/// # Arguments
///
/// * `count_set` Counts of a Julia set in a screen
/// * `png_filename` An output PNG filename
fn draw_image(count_set: CountSet, png_filename: &str) {
    type BitmapCoord = usize;

    let (height, width) = count_set.dim();
    let shape = [height, width, 3];
    let mut bitmap = Bitmap::zeros(shape);
    let max_count_raw = match count_set.iter().max() {
        Some(x) => *x,
        None => 0,
    };
    let max_count = BitmapCoord::try_from(max_count_raw).unwrap();

    let gradient = colorous::CIVIDIS;
    let color_map: Vec<colorous::Color> = (0..=(max_count))
        .map(|i| gradient.eval_rational(i, max_count))
        .collect();

    let mut count_bitmap_r = count_set.mapv(|i| color_map[BitmapCoord::try_from(i).unwrap()].r);
    let count_slice_r = count_bitmap_r.slice_mut(s![.., ..]);
    bitmap.slice_mut(s![.., .., 0]).assign(&count_slice_r);

    let mut count_bitmap_g = count_set.mapv(|i| color_map[BitmapCoord::try_from(i).unwrap()].g);
    let count_slice_g = count_bitmap_g.slice_mut(s![.., ..]);
    bitmap.slice_mut(s![.., .., 1]).assign(&count_slice_g);

    let mut count_bitmap_b = count_set.mapv(|i| color_map[BitmapCoord::try_from(i).unwrap()].b);
    let count_slice_b = count_bitmap_b.slice_mut(s![.., ..]);
    bitmap.slice_mut(s![.., .., 2]).assign(&count_slice_b);

    let raw = bitmap.into_raw_vec();
    let image = RgbImage::from_raw(
        u32::try_from(width).unwrap(),
        u32::try_from(height).unwrap(),
        raw,
    )
    .expect("Creating an image failed");
    image.save(png_filename).expect("Saving an image failed");
}

fn main() {
    let count_set = scan_points(0.382, 0.382, 75, 1024);
    let file = File::create("rust_juliaset.csv").expect("creating file failed");
    let mut writer = WriterBuilder::new().has_headers(false).from_writer(file);
    writer.serialize_array2(&count_set).expect("write failed");
    draw_image(count_set, "rust_juliaset.png");
}

#[test]
fn test_transform_point() {
    let from = Point::new(2.0, 4.0);
    let offset = Point::new(3.0, 5.0);
    let actual = transform_point(from, offset);
    let expected = Point::new(-9.0, 21.0);
    assert_float_eq::assert_float_absolute_eq!(actual.re, expected.re, Coordinate::EPSILON);
    assert_float_eq::assert_float_absolute_eq!(actual.im, expected.im, Coordinate::EPSILON);
}
