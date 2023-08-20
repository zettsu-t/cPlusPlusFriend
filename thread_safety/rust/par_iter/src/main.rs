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

#[derive(Debug, PartialEq)]
pub struct ParamSet {
    n_trials: usize,
    vec_len: usize,
    max_num: Number,
}

impl ParamSet {
    pub fn new(n_trials: usize, vec_len: usize, max_num: Number) -> Self {
        Self {
            n_trials,
            vec_len,
            max_num,
        }
    }
}

#[test]
fn test_param_set() {
    let n_trials = 2;
    let vec_len = 4;
    let max_num = 6;
    let actual = ParamSet::new(n_trials, vec_len, max_num);
    assert_eq!(actual.n_trials, n_trials);
    assert_eq!(actual.vec_len, vec_len);
    assert_eq!(actual.max_num, max_num);
}

fn parse_args(args: Vec<String>) -> ParamSet {
    let mut opts = getopts::Options::new();
    opts.optopt("t", "trial", "# of trials", "# of trials")
        .optopt("s", "size", "The length of an input vector", "Vector size")
        .optopt(
            "m",
            "max",
            "The upper limit of vector elements",
            "Max number",
        );

    let matches = opts.parse(&args[1..]).unwrap();

    let n_trials = match matches.opt_str("t") {
        Some(s) => s.parse::<usize>().unwrap(),
        None => 1,
    };

    let vec_len = match matches.opt_str("s") {
        Some(s) => s.parse::<usize>().unwrap(),
        None => 10,
    };

    let max_num = match matches.opt_str("m") {
        Some(s) => s.parse::<Number>().unwrap(),
        None => 5,
    };

    ParamSet::new(n_trials, vec_len, max_num)
}

#[test]
fn test_default_arguments() {
    let args: Vec<String> = vec!["command"].iter().map(|&s| s.into()).collect();
    let actual = parse_args(args);
    let expected = ParamSet::new(1, 10, 5);
    assert_eq!(actual, expected);
}

#[test]
fn test_short_options() {
    let args: Vec<String> = vec!["command", "-t", "23", "-s", "45", "-m", "67"]
        .iter()
        .map(|&s| s.into())
        .collect();
    let actual = parse_args(args);
    let expected = ParamSet::new(23, 45, 67);
    assert_eq!(actual, expected);
}

#[test]
fn test_long_options() {
    let args: Vec<String> = vec!["command", "--trial", "123", "--size", "456", "--max", "789"]
        .iter()
        .map(|&s| s.into())
        .collect();
    let actual = parse_args(args);
    let expected = ParamSet::new(123, 456, 789);
    assert_eq!(actual, expected);
}

// Run a release build : cargo run --release
fn main() {
    let args: Vec<String> = std::env::args().collect();
    let params = parse_args(args);

    for _ in 0..(params.n_trials) {
        let xs = generate_random_number(params.vec_len, params.max_num);

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
}
