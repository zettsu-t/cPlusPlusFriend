#[cfg(test)]
use juliaset::PixelSize;

use juliaset::draw;
use juliaset::ParamSet;

/// Parses command line arguments and returns their values.
/// Note that opts.parse() can take iterators and we have more
/// generic implementations instead of passing Vec<String> as args.
///
/// # Arguments
///
/// * `args` Command line arguments that includes the name-of-executable
fn parse_args(args: Vec<String>) -> ParamSet {
    let mut opts = getopts::Options::new();
    opts.optopt(
        "x",
        "x_offset",
        "An x offset that is added in iterations",
        "X offset",
    )
    .optopt(
        "y",
        "y_offset",
        "A y offset that is added in iterations",
        "Y offset",
    )
    .optopt("m", "max_iter", "maximum # of iterations", "Max iterations")
    .optopt("s", "size", "# of pixels in the output image", "Pixel size")
    .optopt("c", "csv", "output CSV filename", "CSV filename")
    .optopt("o", "image", "output PNG filename", "PNG filename");
    let matches = opts.parse(&args[1..]).unwrap();

    let x_offset = match matches.opt_str("x") {
        Some(value) => value.parse().unwrap(),
        None => 0.375,
    };

    let y_offset = match matches.opt_str("y") {
        Some(value) => value.parse().unwrap(),
        None => 0.375,
    };

    let max_iter = match matches.opt_str("m") {
        Some(value) => value.parse().unwrap(),
        None => 100,
    };
    assert!(max_iter > 0);

    let n_pixels = match matches.opt_str("s") {
        Some(value) => value.parse().unwrap(),
        None => 256,
    };
    assert!(n_pixels > 1);

    let csv_filename = matches.opt_str("c");
    let filename = matches
        .opt_str("o")
        .unwrap_or_else(|| "rust_juliaset.png".to_string());
    let image_filename = Some(filename);

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
fn test_default_arguments() {
    let args: Vec<String> = vec!["command"].iter().map(|&s| s.into()).collect();
    let actual = parse_args(args);
    let n_pixels: PixelSize = 256;
    let csv_filename = None;
    let image_filename = Some("rust_juliaset.png".to_string());
    let expected = ParamSet::new(0.375, 0.375, 100, n_pixels, &csv_filename, &image_filename);
    assert_eq!(actual, expected);
}

#[test]
fn test_long_arguments() {
    let args: Vec<String> = vec![
        "command",
        "--x_offset",
        "0.25",
        "--y_offset",
        "0.5",
        "--max_iter",
        "16",
        "--size",
        "31",
        "--csv",
        "_test.csv",
        "--image",
        "_test.png",
    ]
    .iter()
    .map(|&s| s.into())
    .collect();
    let actual = parse_args(args);
    let n_pixels: PixelSize = 31;
    let csv_filename = Some("_test.csv".to_string());
    let image_filename = Some("_test.png".to_string());
    let expected = ParamSet::new(0.25, 0.5, 16, n_pixels, &csv_filename, &image_filename);
    assert_eq!(actual, expected);
}

#[test]
fn test_short_arguments() {
    let args: Vec<String> = vec![
        "command",
        "-x",
        "0.5",
        "-y",
        "0.25",
        "-m",
        "31",
        "-s",
        "16",
        "-c",
        "_short.csv",
        "-o",
        "_short.png",
    ]
    .iter()
    .map(|&s| s.into())
    .collect();
    let actual = parse_args(args);
    let n_pixels: PixelSize = 16;
    let csv_filename = Some("_short.csv".to_string());
    let image_filename = Some("_short.png".to_string());
    let expected = ParamSet::new(0.5, 0.25, 31, n_pixels, &csv_filename, &image_filename);
    assert_eq!(actual, expected);
}

#[test]
#[should_panic]
fn test_invalid_x_offset() {
    let args: Vec<String> = vec!["command", "--x_offset", "a.b"]
        .iter()
        .map(|&s| s.into())
        .collect();
    parse_args(args);
}

#[test]
#[should_panic]
fn test_invalid_y_offset() {
    let args: Vec<String> = vec!["command", "--y_offset", "a.b"]
        .iter()
        .map(|&s| s.into())
        .collect();
    parse_args(args);
}

#[test]
#[should_panic]
fn test_invalid_max_iter_not_a_number() {
    let args: Vec<String> = vec!["command", "--max_iter", "a"]
        .iter()
        .map(|&s| s.into())
        .collect();
    parse_args(args);
}

#[test]
#[should_panic]
fn test_invalid_max_iter_negative() {
    let args: Vec<String> = vec!["command", "--max_iter", "-1"]
        .iter()
        .map(|&s| s.into())
        .collect();
    parse_args(args);
}

#[test]
#[should_panic]
fn test_invalid_max_iter_min() {
    let args: Vec<String> = vec!["command", "--max_iter", "0"]
        .iter()
        .map(|&s| s.into())
        .collect();
    parse_args(args);
}

#[test]
#[should_panic]
fn test_invalid_size_not_a_number() {
    let args: Vec<String> = vec!["command", "--size", "a"]
        .iter()
        .map(|&s| s.into())
        .collect();
    parse_args(args);
}

#[test]
#[should_panic]
fn test_invalid_size_negative() {
    let args: Vec<String> = vec!["command", "--size", "-1"]
        .iter()
        .map(|&s| s.into())
        .collect();
    parse_args(args);
}

#[test]
#[should_panic]
fn test_invalid_size_zero() {
    let args: Vec<String> = vec!["command", "--size", "0"]
        .iter()
        .map(|&s| s.into())
        .collect();
    parse_args(args);
}

#[test]
#[should_panic]
fn test_invalid_size_min() {
    let args: Vec<String> = vec!["command", "--size", "1"]
        .iter()
        .map(|&s| s.into())
        .collect();
    parse_args(args);
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let params = parse_args(args);
    draw(&params);
}
