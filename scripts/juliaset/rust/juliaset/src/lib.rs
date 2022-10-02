#[cfg_attr(test, macro_use)]
extern crate assert_float_eq;

#[cfg(test)]
use csv::ReaderBuilder;
#[cfg(test)]
use image::io::Reader as ImageReader;
#[cfg(test)]
use tempfile::Builder;
#[cfg(test)]
use tempfile::TempDir;

// Tests alternative implementations
// #[cfg(test)]
include!("alt.rs");

use csv::WriterBuilder;
use image::RgbImage;
use ndarray::prelude::*;
use ndarray_csv::Array2Writer;
use std::convert::TryFrom;
use std::fs::File;
use std::path::PathBuf;

/// A count that represents how many times a point is transformed
pub type Count = i32;

/// A coordinate in screens
pub type Coordinate = f32;

/// A number of pixels
pub type PixelSize = isize;

/// A point as a set of two coordinates in screens
type Point = num::complex::Complex<Coordinate>;

/// A set of coordinates
type CoordinateSet = ndarray::Array1<Coordinate>;

/// A view for a set of coordinates
type CoordinateSetView<'a> = ndarray::ArrayView1<'a, Coordinate>;

/// RGB colors at pixels in a screen
type Bitmap = ndarray::Array3<u8>;

/// Julia set counts in a screen
type CountSet = ndarray::Array2<Count>;

/// The default tolerance to check if transformations are converged
const DEFAULT_TOLERANCE: Coordinate = 1e-6;
static_assertions::const_assert!(DEFAULT_TOLERANCE > Coordinate::EPSILON);

#[cfg(test)]
type ColorElement = u8;
#[cfg(test)]
const LOW_COLOR_R: ColorElement = 0;
#[cfg(test)]
const LOW_COLOR_G: ColorElement = 32;
#[cfg(test)]
const LOW_COLOR_B: ColorElement = 81;
#[cfg(test)]
const HIGH_COLOR_R: ColorElement = 253;
#[cfg(test)]
const HIGH_COLOR_G: ColorElement = 233;
#[cfg(test)]
const HIGH_COLOR_B: ColorElement = 69;

#[derive(Debug, PartialEq)]
/// A parameter set to draw
pub struct ParamSet {
    /// An x offset that is added in iterations
    x_offset: Coordinate,
    /// A y offset that is added in iterations
    y_offset: Coordinate,
    /// The maximum number of iterations
    max_iter: Count,
    /// The width and height in pixels of an output image
    n_pixels: PixelSize,
    /// A path to save counts as a table
    csv_filepath: Option<PathBuf>,
    /// A path to save counts as an image
    image_filepath: Option<PathBuf>,
}

impl ParamSet {
    /// Creates a new instance
    ///
    /// # Arguments
    ///
    /// * `x_offset` An x offset that is added in iterations
    /// * `y_offset` A y offset that is added in iterations
    /// * `max_iter` The maximum number of iterations
    /// * `n_pixels` The width and height in pixels of an output image
    /// * `csv_filename` A path to save counts as a table
    /// * `image_filename` A path to save counts as an image
    pub fn new(
        x_offset: Coordinate,
        y_offset: Coordinate,
        max_iter: Count,
        n_pixels: PixelSize,
        csv_filename: &Option<String>,
        image_filename: &Option<String>,
    ) -> Self {
        let csv_filepath = csv_filename.as_ref().map(PathBuf::from);
        let image_filepath = image_filename.as_ref().map(PathBuf::from);
        Self {
            x_offset,
            y_offset,
            max_iter,
            n_pixels,
            csv_filepath,
            image_filepath,
        }
    }
}

#[test]
fn test_full_param_set() {
    let x_offset: Coordinate = 0.5;
    let y_offset: Coordinate = 0.125;
    let max_iter: Count = 20;
    let n_pixels: PixelSize = 16;
    let csv_filename = Some("input.csv".to_string());
    let image_filename = Some("input.png".to_string());
    let csv_path = Some(PathBuf::from("input.csv".to_string()));
    let image_path = Some(PathBuf::from("input.png".to_string()));

    let actual = ParamSet::new(
        x_offset,
        y_offset,
        max_iter,
        n_pixels,
        &csv_filename,
        &image_filename,
    );
    assert_float_eq::assert_float_absolute_eq!(actual.x_offset, x_offset, Coordinate::EPSILON);
    assert_float_eq::assert_float_absolute_eq!(actual.y_offset, y_offset, Coordinate::EPSILON);
    assert_eq!(actual.max_iter, max_iter);
    assert_eq!(actual.n_pixels, n_pixels);
    assert_eq!(actual.csv_filepath, csv_path);
    assert_eq!(actual.image_filepath, image_path);
}

#[test]
fn test_partial_param_set() {
    let x_offset: Coordinate = 0.25;
    let y_offset: Coordinate = 0.375;
    let max_iter: Count = 10;
    let n_pixels: PixelSize = 32;
    let csv_filename = None;
    let image_filename = None;

    let actual = ParamSet::new(
        x_offset,
        y_offset,
        max_iter,
        n_pixels,
        &csv_filename,
        &image_filename,
    );
    assert_float_eq::assert_float_absolute_eq!(actual.x_offset, x_offset, Coordinate::EPSILON);
    assert_float_eq::assert_float_absolute_eq!(actual.y_offset, y_offset, Coordinate::EPSILON);
    assert_eq!(actual.max_iter, max_iter);
    assert_eq!(actual.n_pixels, n_pixels);
    assert!(actual.csv_filepath.is_none());
    assert!(actual.image_filepath.is_none());
}

/// Returns a transformed point under the rule of Julia sets
///
/// # Arguments
///
/// * `from` An original point
/// * `offset` An offset to be added
fn transform_point(from: Point, offset: Point) -> Point {
    from * from + offset
}

#[test]
fn test_transform_point() {
    let from = Point::new(2.0, 4.0);
    let offset = Point::new(3.0, 5.0);
    let expected = Point::new(-9.0, 21.0);
    let actual = transform_point(from, offset);
    assert_float_eq::assert_float_absolute_eq!(actual.re, expected.re, Coordinate::EPSILON);
    assert_float_eq::assert_float_absolute_eq!(actual.im, expected.im, Coordinate::EPSILON);
}

/// Returns how many times a point is transformed
///
/// # Arguments
///
/// * `point_x` The x coordinate of a point
/// * `point_y` The y coordinate of a point
/// * `point_offset` An offset to be added to points
/// * `max_iter` The maximum number of iterations
/// * `eps` Tolerance to check if transformations are converged
fn converge_point(
    point_x: Coordinate,
    point_y: Coordinate,
    point_offset: Point,
    max_iter: Count,
    eps: Coordinate,
) -> Count {
    let mut z = Point::new(point_x, point_y);
    let mut count: Count = 0;
    let limit_modulus: Coordinate = 4.0;

    for _ in 0..max_iter {
        let next_z = transform_point(z, point_offset);
        let z_modulus = next_z.norm_sqr();
        if z_modulus > limit_modulus {
            break;
        }

        let delta = (next_z - z).norm_sqr();
        if delta < eps {
            break;
        }

        count += 1;
        z = next_z;
    }

    count
}

#[test]
fn test_converge_point() {
    let eps: Coordinate = Coordinate::EPSILON;
    let zero = Point::new(0.0, 0.0);

    // Min count
    let actual = converge_point(23.0, 0.0, zero, 100, eps);
    assert_eq!(actual, 0 as Count);

    // Eps
    let actual = converge_point(0.9, 0.0, zero, 100, 1e-3);
    assert_eq!(actual, 6 as Count);

    // Converged
    let offset_c = Point::new(0.375, 0.375);
    let actual = converge_point(0.375, 0.375, offset_c, 100, 0.01);
    assert_eq!(actual, 15 as Count);

    // Max count
    let actual = converge_point(0.375, 0.375, offset_c, 11, 0.01);
    assert_eq!(actual, 11 as Count);

    // The machine epsion
    let actual = converge_point(1.0 + eps, 0.0, zero, 100, eps);
    assert_eq!(actual, 0 as Count);

    // Some converged cases
    let offset_a = Point::new(0.5, 0.375);
    let actual = converge_point(0.0, 0.0, offset_a, 100, eps);
    assert_eq!(actual, 4 as Count);

    let offset_b = Point::new(0.375, 0.5);
    let actual = converge_point(0.0, 0.0, offset_b, 100, eps);
    assert_eq!(actual, 8 as Count);

    let actual = converge_point(0.5, 0.375, offset_b, 100, eps);
    assert_eq!(actual, 3 as Count);

    let actual = converge_point(0.375, 0.5, offset_a, 100, eps);
    assert_eq!(actual, 7 as Count);
}

/// Returns how many times each point in a screen is transformed
///
/// # Arguments
///
/// * `xs` A X-coordinates view of points in a screen
/// * `ys` A Y-coordinates view of points in a screen
/// * `point_offset` An offset to be added to points
/// * `max_iter` The maximum number of iterations
/// * `eps` Tolerance to check if transformations are converged
fn converge_point_set(
    xs: CoordinateSetView,
    ys: CoordinateSetView,
    point_offset: Point,
    max_iter: Count,
    eps: Coordinate,
) -> CountSet {
    let mut mat_counts = CountSet::zeros([ys.shape()[0], xs.shape()[0]]);

    for (y_index, point_y) in ys.iter().enumerate() {
        for (x_index, point_x) in xs.iter().enumerate() {
            mat_counts[[y_index, x_index]] =
                converge_point(*point_x, *point_y, point_offset, max_iter, eps);
        }
    }

    mat_counts
}

#[test]
fn test_converge_point_set_row() {
    let xs = CoordinateSet::from_vec(vec![0.375, 0.5]);
    let ys = CoordinateSet::from_vec(vec![0.375]);
    let offset = Point::new(0.375, 0.375);
    let x_view = xs.view();
    let y_view = ys.view();
    let eps: Coordinate = Coordinate::EPSILON;
    let actual = converge_point_set(x_view, y_view, offset, 100, eps);
    assert_eq!(actual.shape()[0], 1);
    assert_eq!(actual.shape()[1], 2);
    assert_eq!(actual[[0, 0]], 15);
    assert_eq!(actual[[0, 1]], 7);
}

#[test]
fn test_converge_point_set_column() {
    let xs = CoordinateSet::from_vec(vec![0.375]);
    let ys = CoordinateSet::from_vec(vec![0.375, 0.5]);
    let offset = Point::new(0.375, 0.375);
    let x_view = xs.view();
    let y_view = ys.view();
    let eps: Coordinate = Coordinate::EPSILON;
    let actual = converge_point_set(x_view, y_view, offset, 100, eps);
    assert_eq!(actual.shape()[0], 2);
    assert_eq!(actual.shape()[1], 1);
    assert_eq!(actual[[0, 0]], 15);
    assert_eq!(actual[[1, 0]], 24);
}

#[test]
fn test_converge_point_set_limit() {
    let xs = CoordinateSet::from_vec(vec![0.375, 0.5]);
    let ys = CoordinateSet::from_vec(vec![0.375]);
    let offset = Point::new(0.5, 0.375);
    let x_view = xs.view();
    let y_view = ys.view();
    let eps: Coordinate = Coordinate::EPSILON;
    let actual = converge_point_set(x_view, y_view, offset, 5, eps);
    assert_eq!(actual.shape()[0], 1);
    assert_eq!(actual.shape()[1], 2);
    assert_eq!(actual[[0, 0]], 5);
    assert_eq!(actual[[0, 1]], 3);
}

/// Returns pixel coordinates on an axis in a screen
///
/// # Arguments
///
/// * `half_length` Maximum x and y coordinates relative to (0,0)
/// * `n_pixels` Numbers of pixels in X and Y axes
fn map_coordinates(half_length: Coordinate, n_pixels: PixelSize) -> CoordinateSet {
    match n_pixels {
        1 => CoordinateSet::zeros([1]),
        n if n < 1 => CoordinateSet::zeros([0]),
        _ => {
            let span = (n_pixels - 1) as Coordinate;
            // Twice the half_length
            (0..n_pixels)
                .map(|i| ((i as Coordinate) * 2.0 * half_length / span) - half_length)
                .collect()
        }
    }
}

#[test]
fn test_map_coordinates_zero() {
    let half_length: Coordinate = 1.0;
    let n_pixels: PixelSize = 0;
    let actual = map_coordinates(half_length, n_pixels);
    assert_eq!(actual.shape()[0], n_pixels as usize);
}

#[test]
fn test_map_coordinates_one() {
    let half_length: Coordinate = 1.0;
    let n_pixels: PixelSize = 1;
    let actual = map_coordinates(half_length, n_pixels);
    assert_eq!(actual.shape()[0], n_pixels as usize);
    assert_float_eq::assert_float_absolute_eq!(actual[0], 0.0, Coordinate::EPSILON);
}

#[test]
fn test_map_coordinates_two() {
    let half_length: Coordinate = 3.0;
    let n_pixels: PixelSize = 2;
    let actual = map_coordinates(half_length, n_pixels);
    let expected = CoordinateSet::from_vec(vec![-3.0, 3.0]);
    assert_eq!(actual, expected);
}

#[test]
fn test_map_coordinates_many() {
    let half_length_odd: Coordinate = 2.0;
    let n_pixels_odd: PixelSize = 5;
    let actual_odd = map_coordinates(half_length_odd, n_pixels_odd);
    let expected_odd = CoordinateSet::from_vec(vec![-2.0, -1.0, 0.0, 1.0, 2.0]);
    assert_eq!(actual_odd, expected_odd);

    let half_length_even: Coordinate = 6.0;
    let n_pixels_even: PixelSize = 4;
    let actual_even = map_coordinates(half_length_even, n_pixels_even);
    let expected_even = CoordinateSet::from_vec(vec![-6.0, -2.0, 2.0, 6.0]);
    assert_eq!(actual_even, expected_even);
}

#[test]
fn test_map_coordinates_negative() {
    let half_length_odd: Coordinate = -2.0;
    let n_pixels_odd: PixelSize = 5;
    let actual_odd = map_coordinates(half_length_odd, n_pixels_odd);
    let expected_odd = CoordinateSet::from_vec(vec![2.0, 1.0, 0.0, -1.0, -2.0]);
    assert_eq!(actual_odd, expected_odd);
}

/// Returns how many times each point in a screen is transformed
///
/// # Arguments
///
/// * `x_offset` An x offset that is added in iterations
/// * `y_offset` A y offset that is added in iterations
/// * `max_iter` The maximum number of iterations
/// * `n_pixels` Numbers of pixels in X and Y axes
fn scan_points(
    x_offset: Coordinate,
    y_offset: Coordinate,
    max_iter: Count,
    n_pixels: PixelSize,
) -> CountSet {
    // The default half width and height of images
    let half_length = (2.0 as Coordinate).sqrt() + 0.1;
    let xs = map_coordinates(half_length, n_pixels);
    let ys = map_coordinates(half_length, n_pixels);
    let point_offset = Point::new(x_offset, y_offset);
    let eps = DEFAULT_TOLERANCE;

    let n_ys = ys.shape()[0];
    crossbeam::scope(|scope| {
        let mut mat_counts = CountSet::zeros([n_ys, xs.shape()[0]]);
        let mut children = vec![];
        let mut y_base: usize = 0;
        let n_cpus = num_cpus::get();
        let x_view = xs.view();
        for ys_sub in ys.axis_chunks_iter(Axis(0), n_ys / n_cpus) {
            children.push(
                scope.spawn(move |_| {
                    converge_point_set(x_view, ys_sub, point_offset, max_iter, eps)
                }),
            );
            y_base += ys_sub.len();
        }
        assert!(y_base == n_ys);

        let mut y_total: usize = 0;
        for child in children {
            let sub_counts = child.join().unwrap();
            let height = sub_counts.shape()[0];
            mat_counts
                .slice_mut(s![y_total..(y_total + height), ..])
                .assign(&sub_counts);
            y_total += height;
        }
        assert!(y_total == n_ys);

        mat_counts
    })
    .unwrap()
}

#[test]
fn test_map_scan_points_capped() {
    let actual_capped = scan_points(0.25, 0.75, 3, 4);
    let expected_capped: CountSet = arr2(&[[0, 0, 1, 0], [0, 2, 3, 0], [0, 3, 2, 0], [0, 1, 0, 0]]);
    assert_eq!(actual_capped, expected_capped);
}

#[test]
fn test_map_scan_points_unlimited() {
    let actual = scan_points(0.25, 0.75, 100, 4);
    let expected: CountSet = arr2(&[[0, 0, 1, 0], [0, 2, 5, 0], [0, 5, 2, 0], [0, 1, 0, 0]]);
    assert_eq!(actual, expected);
}

/// Draws a PNG image from an input screen
///
/// # Arguments
///
/// * `count_set` Counts of a Julia set in a screen
fn draw_image(count_set: &CountSet) -> RgbImage {
    macro_rules! set_color {
        (  $bitmap:expr, $count_set:expr, $color_map:expr, $rgb:tt, $index_t:ty ) => {
            let count_bitmap =
                ($count_set).mapv(|i| $color_map[<$index_t>::try_from(i).unwrap()].$rgb);
            let count_slice = count_bitmap.slice(s![.., ..]);
            $bitmap.assign(&count_slice);
        };
    }

    type BitmapCoord = usize;
    let (height, width) = count_set.dim();
    let shape = [height, width, 3];
    let mut bitmap = Bitmap::zeros(shape);

    type SizeOfColors = usize;
    let max_count_raw = *(count_set.iter().max().unwrap_or(&0));
    let max_count = SizeOfColors::try_from(max_count_raw).unwrap();
    let gradient = colorous::CIVIDIS;
    let color_map: Vec<colorous::Color> = (0..=max_count)
        .map(|i| gradient.eval_rational(i, max_count + 1))
        .collect();

    crossbeam::scope(|scope| {
        let mut children = vec![];

        for (index, mut bitmap_slice) in bitmap.axis_iter_mut(Axis(2)).enumerate() {
            let count_set_view = count_set.view();
            let color_map_view = &color_map[..];
            match index {
                0 => {
                    children.push(scope.spawn(move |_| {
                        set_color!(
                            bitmap_slice,
                            &count_set_view,
                            color_map_view,
                            r,
                            BitmapCoord
                        );
                    }));
                }
                1 => {
                    children.push(scope.spawn(move |_| {
                        set_color!(
                            bitmap_slice,
                            &count_set_view,
                            color_map_view,
                            g,
                            BitmapCoord
                        );
                    }));
                }
                2 => {
                    children.push(scope.spawn(move |_| {
                        set_color!(
                            bitmap_slice,
                            &count_set_view,
                            color_map_view,
                            b,
                            BitmapCoord
                        );
                    }));
                }
                _ => panic!("Unknown color index"),
            }
        }

        for child in children {
            child.join().unwrap();
        }
    })
    .unwrap();

    let raw = bitmap.into_raw_vec();
    RgbImage::from_raw(
        u32::try_from(width).unwrap(),
        u32::try_from(height).unwrap(),
        raw,
    )
    .expect("Creating an image failed")
}

#[test]
fn test_draw_image_monotone() {
    for count in 0..3 {
        let mut count_set: CountSet = arr2(&[[1, 1, 1], [1, 1, 1]]);
        count_set *= count;
        let shape = &count_set.shape();
        let width = u32::try_from(shape[1]).unwrap();
        let height = u32::try_from(shape[0]).unwrap();
        let image = draw_image(&count_set);

        for y in 0..height {
            for x in 0..width {
                let color = image.get_pixel(x, y);
                assert_eq!(color[0], HIGH_COLOR_R);
                assert_eq!(color[1], HIGH_COLOR_G);
                assert_eq!(color[2], HIGH_COLOR_B);
            }
        }
    }
}

#[test]
fn test_draw_image_two_colors() {
    let count_set: CountSet = arr2(&[[0, 0, 0], [1, 0, 0]]);
    let shape = &count_set.shape();
    let width = u32::try_from(shape[1]).unwrap();
    let height = u32::try_from(shape[0]).unwrap();
    let image = draw_image(&count_set);

    for y in 0..height {
        for x in 0..width {
            let color = image.get_pixel(x, y);
            if (x == 0) && (y == 1) {
                assert_eq!(color[0], HIGH_COLOR_R);
                assert_eq!(color[1], HIGH_COLOR_G);
                assert_eq!(color[2], HIGH_COLOR_B);
            } else {
                assert_eq!(color[0], LOW_COLOR_R);
                assert_eq!(color[1], LOW_COLOR_G);
                assert_eq!(color[2], LOW_COLOR_B);
            }
        }
    }
}

#[test]
fn test_draw_image_many_colors() {
    let count_set: CountSet = arr2(&[[0, 4, 8], [16, 32, 64]]);
    let shape = &count_set.shape();
    let width = u32::try_from(shape[1]).unwrap();
    let height = u32::try_from(shape[0]).unwrap();
    let image = draw_image(&count_set);

    let mut prev = 0;
    for y in 0..height {
        for x in 0..width {
            let current = image.get_pixel(x, y)[0];
            if (x == 0) && (y == 0) {
                assert_eq!(current, 0);
            } else {
                assert!(current > prev);
            }
            prev = current;
        }
    }
}

/// Draws a Julia set
///
/// # Arguments
///
/// * `count_set` Julia set counts in a screen
/// * `csv_filepath` A path to save counts as a table
fn write_count(count_set: &CountSet, csv_filepath: &PathBuf) {
    let file = File::create(csv_filepath).expect("Creating a CSV file failed");
    let mut writer = WriterBuilder::new().has_headers(false).from_writer(file);
    writer
        .serialize_array2(&count_set)
        .expect("Writing a CSV file failed");
}

/// Makes a temp dir
#[cfg(test)]
fn make_temp_dir() -> TempDir {
    Builder::new()
        .prefix("test-dir")
        .rand_bytes(10)
        .tempdir()
        .expect("Failed to create a tempdir")
}

#[test]
fn test_write_count() {
    let temp_dir = make_temp_dir();
    let temp_filename = temp_dir.path().join("_test_count_.csv");
    let csv_path = PathBuf::from(temp_filename.to_str().unwrap().to_owned());
    let expeced_n_rows: usize = 2;
    let expeced_n_columns: usize = 3;
    let mut count_set = CountSet::zeros([expeced_n_rows, expeced_n_columns]);
    count_set[[0, 0]] = 1;
    count_set[[0, 1]] = 3;
    count_set[[0, 2]] = 5;
    count_set[[1, 0]] = 20;
    count_set[[1, 1]] = 30;
    count_set[[1, 2]] = 40;

    write_count(&count_set, &csv_path);

    let mut reader = ReaderBuilder::new()
        .has_headers(false)
        .from_path(csv_path)
        .unwrap();
    let mut n_rows: usize = 0;

    for (i, row) in reader.records().enumerate() {
        let cells = row.as_ref().unwrap();
        assert_eq!(cells.len(), expeced_n_columns);
        for (j, cell) in cells.iter().enumerate() {
            assert_eq!(cell, count_set[[i, j]].to_string());
        }
        n_rows = i + 1;
    }
    assert_eq!(n_rows, expeced_n_rows);
}

#[test]
#[should_panic]
fn test_write_count_error() {
    let temp_dir = make_temp_dir();
    let temp_filename = temp_dir.path().join("_test_count_.csv");
    let csv_path = PathBuf::from(temp_filename.to_str().unwrap().to_owned());
    std::fs::remove_dir(temp_dir).unwrap();
    let count_set = CountSet::zeros([3, 4]);
    write_count(&count_set, &csv_path);
}

#[test]
#[should_panic]
fn test_write_count_bad_filename() {
    let temp_dir = make_temp_dir();
    let temp_filename = temp_dir.path().join("..");
    let csv_path = PathBuf::from(temp_filename.to_str().unwrap().to_owned());
    let count_set = CountSet::zeros([3, 4]);
    write_count(&count_set, &csv_path);
}

/// Draws a Julia set
///
/// # Arguments
///
/// * `params` A parameter set to draw
pub fn draw(params: &ParamSet) {
    let count_set = scan_points(
        params.x_offset,
        params.y_offset,
        params.max_iter,
        params.n_pixels,
    );

    match &params.csv_filepath {
        Some(csv_filepath) => {
            write_count(&count_set, &csv_filepath);
        }
        None => (),
    };

    match &params.image_filepath {
        Some(image_filepath) => {
            let image = draw_image(&count_set);
            image.save(image_filepath).expect("Saving an image failed");
        }
        None => (),
    };
}

/// Returns a default parameter set
///
/// # Arguments
///
/// * `csv_filename` A CSV filename to write
/// * `image_filename` A PNG filename to draw
#[cfg(test)]
fn make_default_params(csv_filename: &Option<String>, image_filename: &Option<String>) -> ParamSet {
    let x_offset: Coordinate = 0.5;
    let y_offset: Coordinate = 0.125;
    let max_iter: Count = 20;
    let n_pixels: PixelSize = 16;
    ParamSet::new(
        x_offset,
        y_offset,
        max_iter,
        n_pixels,
        &csv_filename,
        &image_filename,
    )
}

#[test]
fn test_draw() {
    let temp_dir = make_temp_dir();
    let temp_csv_filename = temp_dir.path().join("_test_.csv");
    let temp_png_filename = temp_dir.path().join("_test_.png");
    let csv_filename = Some(temp_csv_filename.to_str().unwrap().to_owned());
    let image_filename = Some(temp_png_filename.to_str().unwrap().to_owned());
    let params = make_default_params(&csv_filename, &image_filename);
    draw(&params);

    let csv_path = Some(PathBuf::from(csv_filename.unwrap()));
    let image_path = Some(PathBuf::from(image_filename.unwrap()));
    let pixel_size = u32::try_from(params.n_pixels).unwrap();

    let mut reader = ReaderBuilder::new()
        .has_headers(false)
        .from_path(csv_path.unwrap())
        .unwrap();
    let mut n_rows: usize = 0;

    for (i, row) in reader.records().enumerate() {
        assert_eq!(row.as_ref().unwrap().len(), pixel_size as usize);
        if i == 0 {
            let cell = &row.as_ref().unwrap()[0];
            assert_eq!(cell, "0");
        }
        // 1-based indexing
        n_rows = i + 1;
    }
    assert_eq!(n_rows, pixel_size as usize);

    let image = ImageReader::open(image_path.unwrap())
        .unwrap()
        .decode()
        .unwrap()
        .to_rgb8();
    assert_eq!(image.width(), pixel_size);
    assert_eq!(image.height(), pixel_size);
    let color = image.get_pixel(0, 0);
    assert_eq!(color[0], LOW_COLOR_R);
    assert_eq!(color[1], LOW_COLOR_G);
    assert_eq!(color[2], LOW_COLOR_B);
}

#[test]
fn test_draw_no_writes() {
    let csv_filename = None;
    let image_filename = None;
    let params = make_default_params(&csv_filename, &image_filename);
    draw(&params);
}

#[test]
#[should_panic]
fn test_draw_csv_error() {
    let temp_dir = make_temp_dir();
    let temp_csv_filename = temp_dir.path().join("_test_.csv");
    let image_filename = None;
    let csv_filename = Some(temp_csv_filename.to_str().unwrap().to_owned());
    let params = make_default_params(&csv_filename, &image_filename);
    std::fs::remove_dir(temp_dir).unwrap();
    draw(&params);
}

#[test]
#[should_panic]
fn test_draw_image_error() {
    let temp_dir = make_temp_dir();
    let csv_filename = None;
    let temp_png_filename = temp_dir.path().join("_test_.png");
    let image_filename = Some(temp_png_filename.to_str().unwrap().to_owned());
    let params = make_default_params(&csv_filename, &image_filename);
    std::fs::remove_dir(temp_dir).unwrap();
    draw(&params);
}

#[test]
#[should_panic]
fn test_draw_bad_csv_filename() {
    let temp_dir = make_temp_dir();
    let temp_csv_filename = temp_dir.path().join("..");
    let temp_png_filename = temp_dir.path().join("_test_.png");
    let csv_filename = Some(temp_csv_filename.to_str().unwrap().to_owned());
    let image_filename = Some(temp_png_filename.to_str().unwrap().to_owned());
    let params = make_default_params(&csv_filename, &image_filename);
    draw(&params);
}

#[test]
#[should_panic]
fn test_draw_bad_png_filename() {
    let temp_dir = make_temp_dir();
    let temp_csv_filename = temp_dir.path().join("_test_.csv");
    let temp_png_filename = temp_dir.path().join("..");
    let csv_filename = Some(temp_csv_filename.to_str().unwrap().to_owned());
    let image_filename = Some(temp_png_filename.to_str().unwrap().to_owned());
    let params = make_default_params(&csv_filename, &image_filename);
    draw(&params);
}
