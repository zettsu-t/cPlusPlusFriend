use criterion::{criterion_group, criterion_main, Criterion};
use juliaset::draw;
use juliaset::ParamSet;

fn bench_full_arguments() {
    let csv_filename = Some("input.csv".to_string());
    let image_filename = Some("input.png".to_string());
    let params = ParamSet::new(0.5, 0.25, 31, 256, &csv_filename, &image_filename);
    draw(&params);
}

fn criterion_benchmark(c: &mut Criterion) {
    c.bench_function("draw", |b| b.iter(|| bench_full_arguments()));
}

criterion_group!(benches, criterion_benchmark);
criterion_main!(benches);
