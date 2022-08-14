#include "juliaset.h"
#include <benchmark/benchmark.h>
#include <iostream>
#include <unistd.h>

static void BM_sample(benchmark::State& state) {
    using namespace juliaset;
    const std::filesystem::path csv_filepath {"bench.csv"};
    const std::filesystem::path png_filepath {"bench.png"};
    constexpr PixelSize n_pixels = 256;
    const ParamSet params(0.5, 0.125, 75, n_pixels, csv_filepath, png_filepath);
    if (draw(params) != ExitStatus::SUCCESS) {
        std::cerr << "!";
        return;
    }
}

BENCHMARK(BM_sample)->Iterations(100);
BENCHMARK_MAIN();
