#[cfg_attr(test, macro_use)]
extern crate assert_float_eq;

extern crate crossbeam;
extern crate getopts;

use csv::WriterBuilder;
use image::RgbImage;
use ndarray::prelude::*;
use ndarray_csv::Array2Writer;
use std::convert::TryFrom;
use std::fs::File;
use std::path::{Path, PathBuf};

/// A count that represents how many times a point is transformed
pub type Count = i32;

/// A coordinate in screens
pub type Coordinate = f32;

/// A number of pixels
pub type PixelSize = isize;

/// A point as two coordinates in screens
type Point = num::complex::Complex<Coordinate>;

/// A set of coordinates and its view
type CoordinateSet = ndarray::Array1<Coordinate>;
type CoordinateSetView<'a> = ndarray::ArrayView1<'a, Coordinate>;

/// RGB colors at pixels in a screen
type Bitmap = ndarray::Array3<u8>;

/// All counts in a screen
type CountSet = ndarray::Array2<Count>;

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
    let mut previous_modulus: Coordinate = 10.0;

    for _ in 0..max_iter {
        z = transform_point(z, point_offset);
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

/// Returns pixel coordinates on an axis in a screen
///
/// # Arguments
///
/// * `half_length` Maximum x and y coordinates relative to (0,0)
/// * `n_pixels` Numbers of pixels in X and Y axes
fn map_coordinates(half_length: Coordinate, n_pixels: PixelSize) -> CoordinateSet {
    if n_pixels < 1 {
        CoordinateSet::zeros([0])
    } else if n_pixels == 1 {
        CoordinateSet::zeros([1])
    } else {
        let n = n_pixels as Coordinate;
        (0..n_pixels)
            .map(|i| ((i as Coordinate) * 2.0 * half_length / (n - 1.0)) - half_length)
            .collect()
    }
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
    let half_length = (2.0 as Coordinate).sqrt() + 0.1;
    let xs = map_coordinates(half_length, n_pixels);
    let ys = map_coordinates(half_length, n_pixels);
    let point_offset = Point::new(x_offset, y_offset);
    let eps = 1e-5;

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

/// Draws a PNG image from an input screen
///
/// # Arguments
///
/// * `count_set` Counts of a Julia set in a screen
/// * `image_filepath` An output PNG filename
fn draw_image(count_set: CountSet, image_filepath: &Path) {
    macro_rules! set_color {
        (  $bitmap:expr, $count_set:expr, $color_map:expr, $rgb:tt, $index_t:ty ) => {
            let count_bitmap =
                ($count_set).mapv(|i| $color_map[<$index_t>::try_from(i).unwrap()].$rgb);
            let count_slice = count_bitmap.slice(s![.., ..]);
            $bitmap.assign(&count_slice);
        };
    }

    println!("Drawing an image");

    type BitmapCoord = usize;
    let (height, width) = count_set.dim();
    let shape = [height, width, 3];
    let mut bitmap = Bitmap::zeros(shape);
    let max_count_raw = *(count_set.iter().max().unwrap_or(&0));
    let max_count = BitmapCoord::try_from(max_count_raw).unwrap();

    let gradient = colorous::CIVIDIS;
    let color_map: Vec<colorous::Color> = (0..=(max_count))
        .map(|i| gradient.eval_rational(i, max_count))
        .collect();

    crossbeam::scope(|scope| {
        let mut children = vec![];

        for (index, mut bitmap_slice) in bitmap.axis_iter_mut(Axis(2)).enumerate() {
            let count_set_view = count_set.view();
            let color_map_view = &color_map[..];
            if index == 0 {
                children.push(scope.spawn(move |_| {
                    set_color!(
                        bitmap_slice,
                        &count_set_view,
                        color_map_view,
                        r,
                        BitmapCoord
                    );
                }));
            } else if index == 1 {
                children.push(scope.spawn(move |_| {
                    set_color!(
                        bitmap_slice,
                        &count_set_view,
                        color_map_view,
                        g,
                        BitmapCoord
                    );
                }));
            } else if index == 2 {
                children.push(scope.spawn(move |_| {
                    set_color!(
                        bitmap_slice,
                        &count_set_view,
                        color_map_view,
                        b,
                        BitmapCoord
                    );
                }));
            } else {
                panic!("Unknown color index")
            }
        }

        for child in children {
            child.join().unwrap();
        }
    })
    .unwrap();

    println!("Writing an image");
    let raw = bitmap.into_raw_vec();
    let image = RgbImage::from_raw(
        u32::try_from(width).unwrap(),
        u32::try_from(height).unwrap(),
        raw,
    )
    .expect("Creating an image failed");
    image.save(image_filepath).expect("Saving an image failed");
}

/// Draws a Julia set
///
/// # Arguments
///
/// * `params` A parameter set to draw
pub fn draw(params: &ParamSet) {
    println!("Scanning points");
    let count_set = scan_points(
        params.x_offset,
        params.y_offset,
        params.max_iter,
        params.n_pixels,
    );

    match &params.csv_filepath {
        Some(csv_filepath) => {
            println!("Writing a CSV file");
            let file = File::create(csv_filepath).expect("Creating a CSV file failed");
            let mut writer = WriterBuilder::new().has_headers(false).from_writer(file);
            writer
                .serialize_array2(&count_set)
                .expect("Writing a CSV file failed");
        }
        None => (),
    };

    match &params.image_filepath {
        Some(image_filepath) => {
            draw_image(count_set, image_filepath);
        }
        None => (),
    };
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

#[test]
fn test_transform_point() {
    let from = Point::new(2.0, 4.0);
    let offset = Point::new(3.0, 5.0);
    let actual = transform_point(from, offset);
    let expected = Point::new(-9.0, 21.0);
    assert_float_eq::assert_float_absolute_eq!(actual.re, expected.re, Coordinate::EPSILON);
    assert_float_eq::assert_float_absolute_eq!(actual.im, expected.im, Coordinate::EPSILON);
}

#[test]
fn test_converge_point() {
    let zero = Point::new(0.0, 0.0);
    let eps: Coordinate = Coordinate::EPSILON;
    let actual = converge_point(23.0, 0.0, zero, 100, eps);
    assert_eq!(actual, 0 as Count);

    let actual = converge_point(1.0 + 1e-7, 0.0, zero, 100, eps);
    assert_eq!(actual, 22 as Count);

    let actual = converge_point(0.0, 1.0 + 1e-7, zero, 11, eps);
    assert_eq!(actual, 11 as Count);

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

    let offset_c = Point::new(0.375, 0.375);
    let actual = converge_point(0.375, 0.375, offset_c, 100, 0.1);
    assert_eq!(actual, 9 as Count);
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
    let half_length: Coordinate = 0.0;
    let n_pixels: PixelSize = 1;
    let actual = map_coordinates(half_length, n_pixels);
    assert_eq!(actual.shape()[0], n_pixels as usize);
    assert_float_eq::assert_float_absolute_eq!(actual[0], -half_length, Coordinate::EPSILON);
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
fn test_map_scan_points() {
    let actual_capped = scan_points(0.25, 0.75, 3, 4) ;
    let expected_capped: CountSet = arr2(&[[0, 0, 1, 0], [0, 2, 3, 0], [0, 3, 2, 0], [0, 1, 0, 0]]);
    assert_eq!(actual_capped, expected_capped);

    let actual = scan_points(0.25, 0.75, 100, 4) ;
    let expected: CountSet = arr2(&[[0, 0, 1, 0], [0, 2, 5, 0], [0, 5, 2, 0], [0, 1, 0, 0]]);
    assert_eq!(actual, expected);
}
