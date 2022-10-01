#[cfg(test)]
/// Parses command line arguments and returns their values using generic
///
/// # Arguments
///
/// * `args` An iterator that feeds command line arguments
fn parse_args_generic<I, T>(args: I) -> ParamSet
where
    I: IntoIterator<Item = T>,
    T: Into<String> + std::convert::AsRef<std::ffi::OsStr> {
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

    // Using an iterator instead of a vector of values
    let matches = opts.parse(args.into_iter().skip(1)).unwrap();

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
fn test_parse_args_generic_string() {
    let args: Vec<String> = vec!["command", "-x", "0.5"]
    .iter()
    .map(|&s| s.into())
    .collect();

    let actual = parse_args_generic(args);
    let expected: Coordinate = 0.5;
    assert_eq!(actual.x_offset, expected);
}

#[test]
fn test_parse_args_generic_env() {
    let actual = parse_args_generic(std::env::args());
    let expected: Coordinate = 0.375;
    assert_eq!(actual.x_offset, expected);
}
