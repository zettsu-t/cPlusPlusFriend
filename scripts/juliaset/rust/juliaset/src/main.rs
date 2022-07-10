use csv::WriterBuilder;
use ndarray_csv::Array2Writer;
use std::fs::File;

type Count = i32;
type Coordinate = f32;
type Point = num::complex::Complex<Coordinate>;
type CoordinateSet = ndarray::Array1<Coordinate>;
type CountSet = ndarray::Array2<Count>;

fn transform_point(z: Point, offset: &Point) -> Point {
    z * z + (*offset)
}

fn converge_point(
    point_x: Coordinate,
    point_y: Coordinate,
    offset: &Point,
    max_iter: Count,
    eps: Coordinate,
) -> Count {
    let mut z = Point::new(point_x, point_y);
    let mut count: Count = 0;
    let mut previous_modulus: Coordinate = 10.0;

    for _ in 0..max_iter {
        z = transform_point(z, &offset);
        let z_modulus = z.norm_sqr();
        if z_modulus > 4.0 {
            break;
        }
        if (previous_modulus - z_modulus).abs() < eps {
            break;
        }

        count = count + 1;
        previous_modulus = z_modulus;
    }

    count
}

fn converge_point_set(
    offset: &Point,
    max_iter: Count,
    eps: Coordinate,
    xs: CoordinateSet,
    ys: CoordinateSet,
) -> CountSet {
    let x_elements = xs.shape()[0];
    let y_elements = ys.shape()[0];
    let shape = [y_elements, x_elements];
    let mut mat_counts = CountSet::zeros(shape);

    for (y_index, point_y) in ys.iter().enumerate() {
        for (x_index, point_x) in xs.iter().enumerate() {
            mat_counts[[y_index, x_index]] =
                converge_point(point_x.clone(), point_y.clone(), &offset, max_iter, eps);
        }
    }

    mat_counts
}

fn map_coordinates(abs_length: Coordinate, n_pixels: isize) -> CoordinateSet {
    let n = n_pixels as Coordinate;
    (0..n_pixels)
        .map(|i| (((i as Coordinate) * 2.0 * abs_length / n) - abs_length))
        .collect()
}

fn scan_points(
    x_offset: Coordinate,
    y_offset: Coordinate,
    max_iter: Count,
    n_pixels: isize,
) -> CountSet {
    let abs_length = (2.0 as Coordinate).sqrt() + 0.1;
    let xs = map_coordinates(abs_length, n_pixels);
    let ys = map_coordinates(abs_length, n_pixels);
    let offset = Point::new(x_offset, y_offset);
    let eps = 1e-5;
    converge_point_set(&offset, max_iter, eps, xs, ys)
}

fn main() {
    let count_set = scan_points(0.382, 0.382, 75, 1024);
    let file = File::create("rust_juliaset.csv").expect("creating file failed");
    let mut writer = WriterBuilder::new().has_headers(false).from_writer(file);
    writer.serialize_array2(&count_set).expect("write failed");
}
