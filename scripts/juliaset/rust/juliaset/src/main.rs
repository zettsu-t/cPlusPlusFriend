#[cfg_attr(test, macro_use)]
extern crate assert_float_eq;

#[cfg(test)]
use assert_cmd::prelude::*;
#[cfg(test)]
use image::io::Reader as ImageReader;
#[cfg(test)]
use predicates::prelude::*;
#[cfg(test)]
use std::process::Command;
#[cfg(test)]
use tempfile::Builder;

extern crate crossbeam;
extern crate getopts;

use csv::WriterBuilder;
use image::RgbImage;
use ndarray::prelude::*;
use ndarray_csv::Array2Writer;
use std::convert::TryFrom;
use std::env;
use std::fs::File;

/// A count that represents how many times a point is transformed
type Count = i32;

/// A coordinate in screens
type Coordinate = f32;

/// A number of pixels
type PixelSize = isize;

/// A point as two coordinates in screens
type Point = num::complex::Complex<Coordinate>;

/// A set of coordinates and its view
type CoordinateSet = ndarray::Array1<Coordinate>;
type CoordinateSetView<'a> = ndarray::ArrayView1<'a, Coordinate>;

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
/// * `png_filename` An output PNG filename
fn draw_image(count_set: CountSet, png_filename: &str) {
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
    let max_count_raw = match count_set.iter().max() {
        Some(x) => *x,
        None => 0,
    };
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

    println!("Writing a PNG image");
    let raw = bitmap.into_raw_vec();
    let image = RgbImage::from_raw(
        u32::try_from(width).unwrap(),
        u32::try_from(height).unwrap(),
        raw,
    )
    .expect("Creating an image failed");
    image.save(png_filename).expect("Saving an image failed");
}

// Takes the first command line argument as a pixel size if available
fn main() {
    let args: Vec<String> = env::args().collect();
    let mut opts = getopts::Options::new();
    opts.optopt("s", "size", "# of pixels in the output image", "Pixel size")
        .optopt("c", "csv", "set output CSV filename", "CSV filename")
        .optopt("o", "image", "set output PNG filename", "PNG filename");
    let matches = opts.parse(&args[1..]).unwrap();

    let n_pixels = match matches.opt_str("s") {
        Some(s) => s.parse::<PixelSize>().unwrap(),
        None => 1024,
    };

    println!("Scanning points");
    let count_set = scan_points(0.382, 0.382, 75, n_pixels);

    match matches.opt_str("c") {
        Some(csv_filename) => {
            println!("Writing a CSV file");
            let file = File::create(csv_filename).expect("Creating a CSV file failed");
            let mut writer = WriterBuilder::new().has_headers(false).from_writer(file);
            writer
                .serialize_array2(&count_set)
                .expect("Writing a CSV file failed");
        }
        None => (),
    };

    let png_filename = match matches.opt_str("o") {
        Some(s) => s,
        None => "rust_juliaset.png".to_string(),
    };
    draw_image(count_set, &png_filename);
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
fn test_invalid_arguments() -> Result<(), Box<dyn std::error::Error>> {
    let mut cmd_size = Command::cargo_bin("juliaset")?;
    cmd_size.arg("-s").arg("0x100").assert().failure();

    let mut cmd_csv = Command::cargo_bin("juliaset")?;
    cmd_csv
        .arg("-s")
        .arg("4")
        .arg("-c")
        .arg("")
        .assert()
        .failure();

    let mut cmd_image = Command::cargo_bin("juliaset")?;
    cmd_image
        .arg("-s")
        .arg("4")
        .arg("-o")
        .arg("foo.txt")
        .assert()
        .failure()
        .stderr(predicate::str::contains("Saving an image failed"));

    let temp_dir = Builder::new().prefix("test-dir").rand_bytes(10).tempdir()?;

    let temp_png_filename = temp_dir.path().join("_test_.png");
    let png_filename = temp_png_filename.to_str().unwrap();

    let mut cmd_success = Command::cargo_bin("juliaset")?;
    let pixel_size: u32 = 16;
    let pixel_size_str = format!("{}", pixel_size);
    cmd_success
        .arg("-s")
        .arg(pixel_size_str)
        .arg("-o")
        .arg(png_filename)
        .assert()
        .success();

    let actual_image = ImageReader::open(png_filename)?.decode()?;
    assert_eq!(actual_image.width(), pixel_size);
    assert_eq!(actual_image.height(), pixel_size);

    Ok(())
}
