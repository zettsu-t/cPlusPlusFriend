extern crate juliaset;

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

#[test]
fn test_wrong_cli_arguments() -> Result<(), Box<dyn std::error::Error>> {
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

    Ok(())
}

#[test]
fn test_correct_cli_arguments() -> Result<(), Box<dyn std::error::Error>> {
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
