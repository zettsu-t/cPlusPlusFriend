extern crate rayon;
use rand::distributions::Uniform;
use rand::Rng;
use rayon::prelude::*;
use std::hint::black_box;
use std::time::Instant;

#[cfg(test)]
use regex::Regex;

type Number = i64;
type Numbers = Vec<Number>;

fn format_duration_msec(start: &std::time::Instant, description: &str) -> String {
    let elapsed = start.elapsed();
    format!("{},{}msec\n", description, elapsed.as_millis())
}

#[test]
fn test_format_duration_msec() {
    let start = Instant::now();
    let actual = format_duration_msec(&start, "Desc");
    let re = Regex::new(r"^Desc,[0-9]+msec\n$").unwrap();
    assert!(re.is_match(&actual));
}

fn generate_random_number(size: usize, max_num: Number) -> Numbers {
    let mut rng = rand::thread_rng();
    let mut xs = vec![0; size];

    for element in xs.iter_mut().take(size) {
        *element = rng.sample(Uniform::new(0, max_num + 1));
    }
    xs
}

#[test]
fn test_generate_random_number() {
    let size = 10000;
    let max_num = 3;
    let actual = generate_random_number(size, max_num);

    assert_eq!(actual.len(), size);
    let result = actual.iter().all(|&x| (x >= 0) && (x <= max_num));
    assert!(result);
}

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
enum Target {
    All,
    Double,
    Sum,
}

#[derive(Debug, PartialEq)]
struct ParamSet {
    n_trials: usize,
    vec_len: usize,
    max_num: Number,
    target: Target,
}

impl ParamSet {
    pub fn new(n_trials: usize, vec_len: usize, max_num: Number, target: Target) -> Self {
        Self {
            n_trials,
            vec_len,
            max_num,
            target,
        }
    }
}

#[test]
fn test_param_set() {
    let n_trials = 2;
    let vec_len = 4;
    let max_num = 6;
    let target = Target::Sum;
    let expected = Target::Sum;

    let actual = ParamSet::new(n_trials, vec_len, max_num, target);
    assert_eq!(actual.n_trials, n_trials);
    assert_eq!(actual.vec_len, vec_len);
    assert_eq!(actual.max_num, max_num);
    assert_eq!(actual.target, expected);
}

fn parse_args(args: Vec<String>) -> ParamSet {
    let mut opts = getopts::Options::new();
    opts.optopt("n", "trial", "# of trials", "# of trials")
        .optopt("s", "size", "The length of an input vector", "Vector size")
        .optopt(
            "m",
            "max",
            "The upper limit of vector elements",
            "Max number",
        )
        .optopt("t", "target", "Targets to measure", "Target");

    let matches = opts.parse(&args[1..]).unwrap();

    let n_trials = match matches.opt_str("n") {
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

    let target = match matches.opt_str("t") {
        Some(s) => match &*s {
            "double" => Target::Double,
            "sum" => Target::Sum,
            _ => Target::All,
        },
        None => Target::All,
    };

    ParamSet::new(n_trials, vec_len, max_num, target)
}

#[test]
fn test_default_arguments() {
    let args: Vec<String> = vec!["command"].iter().map(|&s| s.into()).collect();
    let actual = parse_args(args);
    let expected = ParamSet::new(1, 10, 5, Target::All);
    assert_eq!(actual, expected);
}

#[test]
fn test_short_options() {
    let args: Vec<String> = vec![
        "command", "-n", "23", "-s", "45", "-m", "67", "-t", "double",
    ]
    .iter()
    .map(|&s| s.into())
    .collect();
    let actual = parse_args(args);
    let expected = ParamSet::new(23, 45, 67, Target::Double);
    assert_eq!(actual, expected);
}

#[test]
fn test_long_options() {
    let args: Vec<String> = vec![
        "command", "--trial", "123", "--size", "456", "--max", "789", "--target", "double",
    ]
    .iter()
    .map(|&s| s.into())
    .collect();
    let actual = parse_args(args);
    let expected = ParamSet::new(123, 456, 789, Target::Double);
    assert_eq!(actual, expected);
}

#[test]
fn test_target_options() {
    let args: Vec<String> = vec!["command"].iter().map(|&s| s.into()).collect();
    let actual = parse_args(args);
    assert_eq!(actual.target, Target::All);

    let args: Vec<String> = vec!["command", "--target", "all"]
        .iter()
        .map(|&s| s.into())
        .collect();
    let actual = parse_args(args);
    assert_eq!(actual.target, Target::All);

    let args: Vec<String> = vec!["command", "--target", "double"]
        .iter()
        .map(|&s| s.into())
        .collect();
    let actual = parse_args(args);
    assert_eq!(actual.target, Target::Double);

    let args: Vec<String> = vec!["command", "--target", "sum"]
        .iter()
        .map(|&s| s.into())
        .collect();
    let actual = parse_args(args);
    assert_eq!(actual.target, Target::Sum);
}

fn dispatch(params: &ParamSet) -> String {
    let mut log = String::new();

    for _ in 0..(params.n_trials) {
        let xs = generate_random_number(params.vec_len, params.max_num);

        if (params.target == Target::All) || (params.target == Target::Double) {
            let start_double_seq = Instant::now();
            // Prevent optimized out
            black_box(double_vector_seq(&xs));
            log += &format_duration_msec(&start_double_seq, "double_vector_seq");

            let start_double_par = Instant::now();
            black_box(double_vector_par(&xs));
            log += &format_duration_msec(&start_double_par, "double_vector_par");
        }

        if (params.target == Target::All) || (params.target == Target::Sum) {
            let start_sum_seq = Instant::now();
            black_box(sum_vector_seq(&xs));
            log += &format_duration_msec(&start_sum_seq, "sum_vector_seq");

            let start_sum_par = Instant::now();
            black_box(sum_vector_par(&xs));
            log += &format_duration_msec(&start_sum_par, "sum_vector_par");
        }
    }

    log
}

#[test]
fn test_dispatch() {
    let n_trials = 33;
    let targets = vec![Target::All, Target::Double, Target::Sum];

    for target in targets {
        let n_items = if target == Target::All { 4 } else { 2 };

        let params = ParamSet::new(n_trials, 10, 6, target);
        let re = Regex::new(r"^[^0-9,]+,[0-9]+msec$").unwrap();
        let actual = dispatch(&params);
        let lines: Vec<&str> = actual.lines().collect();

        assert_eq!(n_trials * n_items, lines.len());
        assert!(lines.iter().all(|&line| re.is_match(&line)));

        let n_double_seq = lines
            .iter()
            .filter(|&line| line.starts_with("double_vector_seq"))
            .count();
        let n_double_par = lines
            .iter()
            .filter(|&line| line.starts_with("double_vector_par"))
            .count();
        let n_sum_seq = lines
            .iter()
            .filter(|&line| line.starts_with("sum_vector_seq"))
            .count();
        let n_sum_par = lines
            .iter()
            .filter(|&line| line.starts_with("sum_vector_par"))
            .count();

        let has_double = if (params.target == Target::All) || (params.target == Target::Double) {
            1
        } else {
            0
        };

        let has_sum = if (params.target == Target::All) || (params.target == Target::Sum) {
            1
        } else {
            0
        };

        assert_eq!(n_double_seq, n_trials * has_double);
        assert_eq!(n_double_par, n_trials * has_double);
        assert_eq!(n_sum_seq, n_trials * has_sum);
        assert_eq!(n_sum_par, n_trials * has_sum);
    }
}

// Run a release build : cargo run --release
fn main() {
    let args: Vec<String> = std::env::args().collect();
    let params = parse_args(args);
    let log = dispatch(&params);
    println!("{}", log);
}
