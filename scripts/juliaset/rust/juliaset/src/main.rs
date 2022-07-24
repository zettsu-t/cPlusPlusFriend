#[cfg(test)]
use juliaset::PixelSize;

use juliaset::draw;
use juliaset::ParamSet;
use std::env;

/// Parses command line arguments and returns their values
///
/// # Arguments
///
/// * `args` Command line arguments that includes the name-of-executable
fn parse_args(args: &[String]) -> ParamSet {
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
    .optopt("c", "csv", "set output CSV filename", "CSV filename")
    .optopt("o", "image", "set output PNG filename", "PNG filename");
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

    let n_pixels = match matches.opt_str("s") {
        Some(value) => value.parse().unwrap(),
        None => 256,
    };

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
fn test_full_arguments() {
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
        "input.csv",
        "-o",
        "input.png",
    ]
    .iter()
    .map(|&s| s.into())
    .collect();
    let actual = parse_args(&args);
    let n_pixels: PixelSize = 16;
    let csv_filename = Some("input.csv".to_string());
    let image_filename = Some("input.png".to_string());
    let expected = ParamSet::new(0.5, 0.25, 31, n_pixels, &csv_filename, &image_filename);
    assert_eq!(actual, expected);
}

#[test]
fn test_default_arguments() {
    let args: Vec<String> = vec!["command"].iter().map(|&s| s.into()).collect();
    let actual = parse_args(&args);
    let n_pixels: PixelSize = 256;
    let csv_filename = None;
    let image_filename = Some("rust_juliaset.png".to_string());
    let expected = ParamSet::new(0.375, 0.375, 100, n_pixels, &csv_filename, &image_filename);
    assert_eq!(actual, expected);
}

// Takes the first command line argument as a pixel size if available
fn main() {
    let args: Vec<String> = env::args().collect();
    let params = parse_args(&args);
    draw(&params);
}
