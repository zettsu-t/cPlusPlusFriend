extern crate rayon;
use rand::distributions::Uniform;
use rand::Rng;
use rayon::prelude::*;
use std::time::Instant;

type Number = i64;
type Numbers = Vec<Number>;

fn double_vector_seq(xs: &Numbers) -> Numbers {
    xs.iter().map(|&x| x * 2).collect()
}

fn double_vector_par(xs: &Numbers) -> Numbers {
    xs.par_iter().map(|&x| x * 2).collect()
}

fn sum_vector_seq(xs: &Numbers) -> Number {
    xs.iter().map(|&x| x * 2).filter(|&x| x % 3 == 0).sum()
}

fn sum_vector_par(xs: &Numbers) -> Number {
    xs.par_iter().map(|&x| x * 2).filter(|&x| x % 3 == 0).sum()
}

fn generate_random_number(size: usize, max_num: Number) -> Numbers {
    let mut rng = rand::thread_rng();
    let mut xs = vec![0; size];

    for element in xs.iter_mut().take(size) {
        *element = rng.sample(Uniform::new(0, max_num + 1));
    }
    xs
}

fn print_duration_msec(start: &std::time::Instant) {
    let elapsed = start.elapsed();
    println!("{} ms", elapsed.as_millis());
}

#[test]
fn test_double_vector() {
    let xs = vec![5, 6, 7];
    let expected = vec![10, 12, 14];

    let actual_seq = double_vector_seq(&xs);
    assert_eq!(actual_seq, expected);

    let actual_par = double_vector_par(&xs);
    assert_eq!(actual_par, expected);
}

#[test]
fn test_sum_vector() {
    let xs = vec![5, 6, 7, 10, 15];
    let expected = 42;

    let actual_seq = sum_vector_seq(&xs);
    assert_eq!(actual_seq, expected);

    let actual_par = sum_vector_par(&xs);
    assert_eq!(actual_par, expected);
}

#[test]
fn test_generate_random_number() {
    let size = 10000;
    let max_num = 9;
    let actual = generate_random_number(size, max_num);

    assert_eq!(actual.len(), size);
    let result = actual.iter().all(|&x| (x >= 0) && (x <= max_num));
    assert!(result);
}

#[test]
fn test_double_vector_random() {
    for _ in 0..100 {
        let size = 10000;
        let max_num = 9;
        let xs = generate_random_number(size, max_num);

        let actual_seq = double_vector_seq(&xs);
        let actual_par = double_vector_par(&xs);
        assert_eq!(actual_seq, actual_par);
    }
}

#[test]
fn test_sum_vector_random() {
    for _ in 0..100 {
        let size = 10000;
        let max_num = 99;
        let xs = generate_random_number(size, max_num);

        let sum_seq = sum_vector_seq(&xs);
        let sum_par = sum_vector_par(&xs);
        assert_eq!(sum_seq, sum_par);
        assert_eq!(sum_seq % 2, 0);
        assert_eq!(sum_seq % 3, 0);
    }
}

// Run a release build : cargo run --release
fn main() {
    let size = 100000000;
    let max_num = 2000;
    let xs = generate_random_number(size, max_num);

    let start_double_seq = Instant::now();
    let ys_seq = double_vector_seq(&xs);
    print_duration_msec(&start_double_seq);

    let start_sum_seq = Instant::now();
    let sum_seq = sum_vector_seq(&ys_seq);
    print_duration_msec(&start_sum_seq);
    // Prevent optimized out
    println!("{}", sum_seq);

    let start_double_par = Instant::now();
    let ys_par = double_vector_par(&xs);
    print_duration_msec(&start_double_par);

    let start_sum_par = Instant::now();
    let sum_par = sum_vector_par(&ys_par);
    print_duration_msec(&start_sum_par);
    println!("{}", sum_par);
}
