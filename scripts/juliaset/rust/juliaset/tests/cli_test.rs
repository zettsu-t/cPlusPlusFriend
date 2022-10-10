extern crate juliaset;

#[cfg(test)]
use assert_cmd::prelude::*;
#[cfg(test)]
use csv::ReaderBuilder;
#[cfg(test)]
use image::io::Reader as ImageReader;
#[cfg(test)]
use predicates::prelude::*;
#[cfg(test)]
use std::path::Path;
#[cfg(test)]
use std::process::Command;
#[cfg(test)]
use tempfile::Builder;

#[test]
fn test_wrong_cli_arguments() -> Result<(), Box<dyn std::error::Error>> {
    let mut cmd_size = Command::cargo_bin("appjuliaset")?;
    cmd_size.arg("-s").arg("0x100").assert().failure();

    let mut cmd_csv = Command::cargo_bin("appjuliaset")?;
    cmd_csv
        .arg("-s")
        .arg("4")
        .arg("-c")
        .arg("")
        .assert()
        .failure();

    let mut cmd_image = Command::cargo_bin("appjuliaset")?;
    cmd_image
        .arg("-s")
        .arg("4")
        .arg("-o")
        .arg("foo.txt")
        .assert()
        .failure()
        .stderr(predicate::str::contains("Saving an image failed"));

    Ok(())
}

/// Counts how many files exist that matches a glob
///
/// # Arguments
///
/// * `dir` A target directory glob
/// * `file_pattern` A filename glob
#[cfg(test)]
fn count_temp_dir(dir: &Path, file_pattern: &str) -> usize {
    let file_glob = dir.join(file_pattern);
    let pattern = file_glob.into_os_string().into_string().unwrap();
    glob::glob(&pattern).unwrap().filter(|x| x.is_ok()).count()
}

#[test]
fn test_correct_csv_arguments() -> Result<(), Box<dyn std::error::Error>> {
    let temp_dir = Builder::new().prefix("test-dir").rand_bytes(10).tempdir()?;
    let temp_csv_filename = temp_dir.path().join("_test_.csv");
    let csv_filename = temp_csv_filename.to_str().unwrap();

    let mut cmd_success = Command::cargo_bin("appjuliaset")?;
    let pixel_size: u32 = 8;
    let pixel_size_str = format!("{}", pixel_size);
    cmd_success
        .arg("-x")
        .arg("-0.875")
        .arg("-y")
        .arg("0.25")
        .arg("-m")
        .arg("20")
        .arg("-s")
        .arg(pixel_size_str)
        .arg("-c")
        .arg(csv_filename)
        .assert()
        .success();

    let n_csvfiles = count_temp_dir(temp_dir.path(), "*.csv");
    assert_eq!(n_csvfiles, 1 as usize);
    let n_pngfiles = count_temp_dir(temp_dir.path(), "*.png");
    assert_eq!(n_pngfiles, 0 as usize);

    let mut reader = ReaderBuilder::new()
        .has_headers(false)
        .from_path(csv_filename)?;
    let mut n_columns: usize = 0;

    type Count = i32;
    let expected_max: Count = 20;
    let mut actual_max: Count = 0;

    for (i, row) in reader.records().enumerate() {
        let cells: Vec<Count> = row?.iter().map(|s| s.parse().unwrap()).collect();
        assert_eq!(cells.len(), usize::try_from(pixel_size).unwrap());
        let cell_sum = cells.iter().sum::<Count>();
        let cell_max = *(cells.iter().max().unwrap_or(&0));
        actual_max = std::cmp::max(actual_max, cell_max);

        if (i == 0) | ((i + 1) == pixel_size.try_into().unwrap()) {
            // A row of all 0s
            assert_eq!(0, cell_sum);
        } else {
            assert!(cell_sum > 0);
        }

        // 1-based indexing
        n_columns = i + 1;
    }

    assert_eq!(actual_max, expected_max);
    assert_eq!(n_columns, usize::try_from(pixel_size).unwrap());
    Ok(())
}

#[test]
fn test_correct_image_arguments() -> Result<(), Box<dyn std::error::Error>> {
    let temp_dir = Builder::new().prefix("test-dir").rand_bytes(10).tempdir()?;
    let temp_png_filename = temp_dir.path().join("_test_.png");
    let png_filename = temp_png_filename.to_str().unwrap();

    let mut cmd_success = Command::cargo_bin("appjuliaset")?;
    let pixel_size: u32 = 16;
    let pixel_size_str = format!("{}", pixel_size);
    cmd_success
        .arg("-s")
        .arg(pixel_size_str)
        .arg("-o")
        .arg(png_filename)
        .assert()
        .success();

    let n_csvfiles = count_temp_dir(temp_dir.path(), "*.csv");
    assert_eq!(n_csvfiles, 0 as usize);
    let n_pngfiles = count_temp_dir(temp_dir.path(), "*.png");
    assert_eq!(n_pngfiles, 1 as usize);

    let actual_image = ImageReader::open(png_filename)?.decode()?;
    assert_eq!(actual_image.width(), pixel_size);
    assert_eq!(actual_image.height(), pixel_size);
    // A non-zero color found
    assert!(!actual_image.to_rgb8().iter().find(|&x| x > &0).is_none());

    Ok(())
}
