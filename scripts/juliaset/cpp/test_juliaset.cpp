#include "juliaset.h"
#include <fstream>
#include <gtest/gtest.h>
#include <random>
#include <tuple>

using namespace juliaset;

namespace juliaset {
namespace testing {
/// A immutable array view for a 1-D coordinate set
using CoordinateSetRef = boost::const_multi_array_ref<Coordinate, 1>;

/// A vector of counts that works with initializer lists
using CountVector = std::vector<Count>;

/// A vector of vectors that works with initializer lists
using Count2dVector = std::vector<std::vector<Count>>;

/**
 * @brief Copies a vector to a row in a 2-D array
 * @param[in] src A source vector
 * @param[in] dst A destination 2-D array
 * @param[in] y A row number in the array
 */
void copy_array(const CountVector& src, CountSet& dst, PixelSize y) {
    boost::const_multi_array_ref<Count, 1> row{src.data(), boost::extents[src.size()]};
    CountSet::array_view<1>::type y_view = dst[boost::indices[y][CountSet::index_range()]];
    y_view = row;
    return;
}

/**
 * @brief Copies a vector of vectors to a 2-D array
 * @param[in] src A vector of vectors
 * @param[in] dst A destination 2-D array
 */
void copy_2drray(const Count2dVector& src, CountSet& dst) {
    PixelSize i = 0;
    for (const auto& row : src) {
        copy_array(row, dst, i);
        ++i;
    }
    return;
}

/**
 * @brief Makes arguments for parse_command_line()
 * @param[in] arg_set An array of command line arguments
 * @return The argc and argv
 */
std::tuple<int, std::vector<const char*>> make_argc_argv(const std::vector<std::string>& arg_set) {
    std::vector<const char*> argv;
    for (const auto& arg : arg_set) {
        argv.push_back(arg.c_str());
    }

    auto argc = checked_cast<int>(argv.size());
    argv.push_back(nullptr);
    return std::make_tuple(argc, argv);
}

/**
 * A temporary file that only lives in a testing process.
 * Each instance of TempFile creates a temporary directory
 * and a temporary filename in the directory.
 */
class TempFile {
  public:
    /**
     * @param[in] extension A trailing string that possibly starts with a dot
     */
    TempFile(const std::string& extension) {
        using RandomNumber = double;
        std::random_device seed_gen;
        std::mt19937 gen(seed_gen());
        std::uniform_real_distribution<RandomNumber> dist(0.0, 1.0);
        const auto tempdir_top = std::filesystem::temp_directory_path();
        if (!std::filesystem::exists(tempdir_top)) {
            return;
        }

        auto make_random_digits = [&]() {
            const auto value = static_cast<long long>(dist(gen) * 1'000'000'000'000'000ll);
            return std::to_string(value);
        };

        constexpr int max_trial = 10;
        for (int trial = 0; trial < max_trial; ++trial) {
            if (topdir_.has_value() || filepath_.has_value()) {
                break;
            }

            std::string subdir = "_temp_" + make_random_digits();
            auto temp_dir = tempdir_top;
            temp_dir /= subdir;

            if (std::filesystem::exists(temp_dir)) {
                continue;
            }

            std::filesystem::create_directory(temp_dir);
            if (!std::filesystem::is_directory(temp_dir)) {
                continue;
            }
            topdir_ = temp_dir;

            std::string filename = "_" + make_random_digits();
            filename += extension;
            auto temp_file = temp_dir;
            temp_file /= filename;

            if (!std::filesystem::exists(temp_file)) {
                filepath_ = temp_file;
            }
            break;
        }
    }

    virtual ~TempFile() {
        clean();
    }

    /**
     * @brief Returns a reference to a temporary filename
     * @return A temporary filename if available
     */
    const std::optional<std::filesystem::path>& Get() const { return filepath_; }

    /**
     * @brief Causes write errors in testing
     */
    virtual void CauseError() {
        clean();
    }

  private:
    void clean() {
        if (filepath_.has_value()) {
            if (std::filesystem::exists(filepath_.value())) {
                std::filesystem::remove(filepath_.value());
            }
            filepath_.reset();
        }

        if (topdir_.has_value()) {
            if (std::filesystem::is_directory(topdir_.value())) {
                std::filesystem::remove(topdir_.value());
            }
            topdir_.reset();
        }
    }

    /// A temporary directory that exists
    std::optional<std::filesystem::path> topdir_;
    /// A temporary filename
    std::optional<std::filesystem::path> filepath_;
};
}} // namespace

using namespace juliaset::testing;

class TestTempFile : public ::testing::Test {};

TEST_F(TestTempFile, All) {
    const std::string extension{".csv"};
    std::filesystem::path csv_filepath;
    {
        TempFile csv(extension);
        const auto& actual = csv.Get();
        ASSERT_TRUE(actual.has_value());
        csv_filepath = actual.value();
    }

    const auto filename = csv_filepath.string();
    ASSERT_NE(std::string::npos, filename.find(".csv"));
    ASSERT_GT(filename.size(), extension.size());
    ASSERT_EQ(filename.size() - extension.size(), filename.rfind(".csv"));
    ASSERT_FALSE(std::filesystem::exists(csv_filepath));
    ASSERT_FALSE(std::filesystem::exists(csv_filepath.parent_path()));
    ASSERT_TRUE(std::filesystem::exists(csv_filepath.parent_path().parent_path()));
}

TEST_F(TestTempFile, RemoveDir) {
    const std::string extension{".csv"};
    TempFile csv(extension);
    const auto actual = csv.Get();
    ASSERT_TRUE(actual.has_value());
    const auto csv_dir = actual.value().parent_path();

    csv.CauseError();
    ASSERT_FALSE(std::filesystem::is_directory(csv_dir));
}

TEST_F(TestTempFile, RemoveFile) {
    const std::string extension{".csv"};
    TempFile csv(extension);
    const auto actual = csv.Get();
    ASSERT_TRUE(actual.has_value());
    const auto csv_filepath = actual.value();

    {
        std::ofstream os(csv_filepath);
    }

    ASSERT_TRUE(std::filesystem::exists(csv_filepath));
    csv.CauseError();
    ASSERT_FALSE(std::filesystem::exists(csv_filepath));
    ASSERT_FALSE(std::filesystem::is_directory(csv_filepath.parent_path()));
}

class TestParamSet : public ::testing::Test {};

TEST_F(TestParamSet, Full) {
    constexpr Coordinate x_offset = 0.5;
    constexpr Coordinate y_offset = 0.125;
    constexpr Count max_iter = 20;
    constexpr PixelSize n_pixels = 16;
    const std::string csv_filename{"input.csv"};
    const std::string image_filename{"input.png"};
    const std::optional<std::filesystem::path> csv_path{"input.csv"};
    const std::optional<std::filesystem::path> image_path{"input.png"};

    const auto actual =
        ParamSet(x_offset, y_offset, max_iter, n_pixels, csv_filename, image_filename);

    EXPECT_FLOAT_EQ(x_offset, actual.x_offset);
    EXPECT_FLOAT_EQ(y_offset, actual.y_offset);
    EXPECT_EQ(max_iter, actual.max_iter);
    EXPECT_EQ(n_pixels, actual.n_pixels);
    EXPECT_EQ(csv_path.value(), actual.csv_filepath.value());
    EXPECT_EQ(image_path.value(), actual.image_filepath.value());
}

TEST_F(TestParamSet, Partial) {
    constexpr Coordinate x_offset = 0.25;
    constexpr Coordinate y_offset = 0.375;
    constexpr Count max_iter = 10;
    constexpr PixelSize n_pixels = 32;
    const std::optional<std::string> csv_filename;
    const std::optional<std::string> image_filename;
    const auto actual =
        ParamSet(x_offset, y_offset, max_iter, n_pixels, csv_filename, image_filename);

    EXPECT_FLOAT_EQ(x_offset, actual.x_offset);
    EXPECT_FLOAT_EQ(y_offset, actual.y_offset);
    EXPECT_EQ(max_iter, actual.max_iter);
    EXPECT_EQ(n_pixels, actual.n_pixels);
    EXPECT_FALSE(actual.csv_filepath.has_value());
    EXPECT_FALSE(actual.image_filepath.has_value());
}

class TestComplex : public ::testing::Test {};

TEST_F(TestComplex, NormSqr) {
    constexpr Point left_top{3.0, 4.0};
    EXPECT_FLOAT_EQ(25.0, norm2_sqr(left_top));

    constexpr Point right_bottom{-2.0, -3.0};
    EXPECT_FLOAT_EQ(13.0, norm2_sqr(right_bottom));
}

class TestTransformPoint : public ::testing::Test {};

TEST_F(TestTransformPoint, All) {
    constexpr Point from{2.0, 4.0};
    constexpr Point offset{3.0, 5.0};
    constexpr Point expected{-9.0, 21.0};
    const auto actual = transform_point(from, offset);
    EXPECT_FLOAT_EQ(expected.real(), actual.real());
    EXPECT_FLOAT_EQ(expected.imag(), actual.imag());
}

class TestConvergePoint : public ::testing::Test {};

TEST_F(TestConvergePoint, All) {
    constexpr Coordinate eps = std::numeric_limits<Coordinate>::epsilon();
    constexpr auto delta = static_cast<Coordinate>(1e-7f);

    const Point zero{0.0, 0.0};
    const auto actual_zero = converge_point(23.0, 0.0, zero, 100, eps);
    EXPECT_EQ(0, actual_zero);

    const auto actual_real = converge_point(1 + delta, 0.0, zero, 100, eps);
    EXPECT_EQ(22, actual_real);

    const auto actual_imag = converge_point(0.0, 1 + delta, zero, 11, eps);
    EXPECT_EQ(11, actual_imag);

    const Point offset_a{0.5, 0.375};
    const auto actual_offset_a = converge_point(0.0, 0.0, offset_a, 100, eps);
    EXPECT_EQ(4, actual_offset_a);

    const Point offset_b{0.375, 0.5};
    const auto actual_offset_b = converge_point(0.0, 0.0, offset_b, 100, eps);
    EXPECT_EQ(8, actual_offset_b);

    const auto actual_all_a = converge_point(0.5, 0.375, offset_b, 100, eps);
    EXPECT_EQ(3, actual_all_a);

    const auto actual_all_b = converge_point(0.375, 0.5, offset_a, 100, eps);
    EXPECT_EQ(7, actual_all_b);

    const Point offset_c{0.375, 0.375};
    const auto actual_all_c = converge_point(0.375, 0.375, offset_c, 100, 0.1f);
    EXPECT_EQ(9, actual_all_c);
}

class TestConvergePointSet : public ::testing::Test {};

TEST_F(TestConvergePointSet, Row) {
    CoordinateSet xs(boost::extents[2]);
    CoordinateSet ys(boost::extents[1]);
    xs[0] = 0.375;
    xs[1] = 0.5;
    ys[0] = 0.375;

    const Point offset{0.375, 0.375};
    CoordinateSetView view_xs = xs[boost::indices[decltype(xs)::index_range()]];
    CoordinateSetView view_ys = ys[boost::indices[decltype(ys)::index_range()]];
    const auto actual = converge_point_set(view_xs, view_ys, offset, 100, DefaultEps);
    ASSERT_EQ(1, actual.shape()[0]);
    ASSERT_EQ(2, actual.shape()[1]);
    EXPECT_EQ(15, actual[0][0]);
    EXPECT_EQ(7, actual[0][1]);
}

TEST_F(TestConvergePointSet, Column) {
    CoordinateSet xs(boost::extents[1]);
    CoordinateSet ys(boost::extents[2]);
    xs[0] = 0.375;
    ys[0] = 0.375;
    ys[1] = 0.5;

    const Point offset{0.375, 0.375};
    CoordinateSetView view_xs = xs[boost::indices[decltype(xs)::index_range()]];
    CoordinateSetView view_ys = ys[boost::indices[decltype(ys)::index_range()]];
    const auto actual = converge_point_set(view_xs, view_ys, offset, 100, DefaultEps);
    ASSERT_EQ(2, actual.shape()[0]);
    ASSERT_EQ(1, actual.shape()[1]);
    EXPECT_EQ(15, actual[0][0]);
    EXPECT_EQ(24, actual[1][0]);
}

TEST_F(TestConvergePointSet, Limit) {
    CoordinateSet xs(boost::extents[2]);
    CoordinateSet ys(boost::extents[1]);
    xs[0] = 0.375;
    xs[1] = 0.5;
    ys[0] = 0.375;

    const Point offset{0.5, 0.375};
    CoordinateSetView view_xs = xs[boost::indices[decltype(xs)::index_range()]];
    CoordinateSetView view_ys = ys[boost::indices[decltype(ys)::index_range()]];
    const auto actual = converge_point_set(view_xs, view_ys, offset, 5, DefaultEps);
    ASSERT_EQ(1, actual.shape()[0]);
    ASSERT_EQ(2, actual.shape()[1]);
    EXPECT_EQ(5, actual[0][0]);
    EXPECT_EQ(3, actual[0][1]);
}

class TestMapCoordinates : public ::testing::Test {};

TEST_F(TestMapCoordinates, Zero) {
    constexpr Coordinate half_length = 1;
    constexpr PixelSize n_pixels = 0;
    const auto actual = map_coordinates(half_length, n_pixels);
    ASSERT_EQ(n_pixels, actual.shape()[0]);
}

TEST_F(TestMapCoordinates, One) {
    constexpr Coordinate half_length = 0;
    constexpr PixelSize n_pixels = 1;
    const auto actual = map_coordinates(half_length, n_pixels);
    ASSERT_EQ(n_pixels, actual.shape()[0]);
    EXPECT_FLOAT_EQ(-half_length, actual[0]);
}

TEST_F(TestMapCoordinates, Two) {
    constexpr Coordinate half_length = 3;
    constexpr PixelSize n_pixels = 2;
    const auto actual = map_coordinates(half_length, n_pixels);
    ASSERT_EQ(n_pixels, actual.shape()[0]);

    const std::vector<CoordinateSet::value_type> expected_vec{-3.0, 3.0};
    CoordinateSetRef expected{expected_vec.data(), boost::extents[expected_vec.size()]};
    ASSERT_EQ(n_pixels, expected.size());
    ASSERT_EQ(expected, actual);
}

TEST_F(TestMapCoordinates, Many) {
    constexpr Coordinate half_length_odd = 2;
    constexpr PixelSize n_pixels_odd = 5;
    const auto actual_odd = map_coordinates(half_length_odd, n_pixels_odd);
    const std::vector<CoordinateSet::value_type> expected_odd_vec{-2.0, -1.0, 0.0, 1.0, 2.0};

    CoordinateSetRef expected_odd{expected_odd_vec.data(), boost::extents[expected_odd_vec.size()]};
    ASSERT_EQ(n_pixels_odd, expected_odd.size());
    ASSERT_EQ(expected_odd, actual_odd);

    constexpr Coordinate half_length_even = 6;
    constexpr PixelSize n_pixels_even = 4;
    const auto actual_even = map_coordinates(half_length_even, n_pixels_even);
    const std::vector<CoordinateSet::value_type> expected_even_vec{-6.0, -2.0, 2.0, 6.0};
    CoordinateSetRef expected_even{expected_even_vec.data(),
                                   boost::extents[expected_even_vec.size()]};
    ASSERT_EQ(n_pixels_even, expected_even.size());
    ASSERT_EQ(expected_even, actual_even);
}

class TestScanPoints : public ::testing::Test {};

TEST_F(TestScanPoints, Capped) {
    constexpr PixelSize n_pixels = 4;
    const Count2dVector count_set{{0, 0, 1, 0}, {0, 2, 3, 0}, {0, 3, 2, 0}, {0, 1, 0, 0}};
    CountSet expected(boost::extents[n_pixels][n_pixels]);
    copy_2drray(count_set, expected);

    const auto actual = scan_points(0.25, 0.75, 3, n_pixels);
    EXPECT_EQ(actual, expected);
}

TEST_F(TestScanPoints, Unlimited) {
    constexpr PixelSize n_pixels = 4;
    const Count2dVector count_set{{0, 0, 1, 0}, {0, 2, 5, 0}, {0, 5, 2, 0}, {0, 1, 0, 0}};
    CountSet expected(boost::extents[n_pixels][n_pixels]);
    copy_2drray(count_set, expected);

    const auto actual = scan_points(0.25, 0.75, 100, n_pixels);
    EXPECT_EQ(actual, expected);
}

class TestMakeGradientColors : public ::testing::Test {};

TEST_F(TestMakeGradientColors, One) {
    const auto table = make_gradient_colors(0);
    ASSERT_EQ(1, table.size());

    const auto pixel = table.at(0);
    EXPECT_EQ(HIGH_COLOR_R, boost::gil::at_c<0>(pixel));
    EXPECT_EQ(HIGH_COLOR_G, boost::gil::at_c<1>(pixel));
    EXPECT_EQ(HIGH_COLOR_B, boost::gil::at_c<2>(pixel));
}

TEST_F(TestMakeGradientColors, TwoThree) {
    for (Count max_count{1}; max_count <= 2; ++max_count) {
        const auto table = make_gradient_colors(max_count);
        ASSERT_EQ(max_count + 1, table.size());

        const auto pixel_low = table.at(0);
        EXPECT_EQ(LOW_COLOR_R, boost::gil::at_c<0>(pixel_low));
        EXPECT_EQ(LOW_COLOR_G, boost::gil::at_c<1>(pixel_low));
        EXPECT_EQ(LOW_COLOR_B, boost::gil::at_c<2>(pixel_low));

        const auto pixel_high = table.at(max_count);
        EXPECT_EQ(HIGH_COLOR_R, boost::gil::at_c<0>(pixel_high));
        EXPECT_EQ(HIGH_COLOR_G, boost::gil::at_c<1>(pixel_high));
        EXPECT_EQ(HIGH_COLOR_B, boost::gil::at_c<2>(pixel_high));

        if (max_count > 1) {
            const auto pixel_mid = table.at(1);
            auto mid_point = [](ColorElement low, ColorElement high) {
                using ColorValue = double;
                return static_cast<ColorElement>(
                    (static_cast<ColorValue>(low) + static_cast<ColorValue>(high)) / 2.0);
            };

            EXPECT_EQ(mid_point(LOW_COLOR_R, HIGH_COLOR_R), boost::gil::at_c<0>(pixel_mid));
            EXPECT_EQ(mid_point(LOW_COLOR_G, HIGH_COLOR_G), boost::gil::at_c<1>(pixel_mid));
            EXPECT_EQ(mid_point(LOW_COLOR_B, HIGH_COLOR_B), boost::gil::at_c<2>(pixel_mid));
        }
    }
}

class TestDrawImage : public ::testing::Test {};

TEST_F(TestDrawImage, Monotone) {
    for (Count count = 0; count < 3; ++count) {
        CountSet count_set(boost::extents[2][3]);
        auto n_ys = count_set.shape()[0];
        auto n_xs = count_set.shape()[1];

        for (decltype(n_ys) y{0}; y < n_ys; ++y) {
            for (decltype(n_xs) x{0}; x < n_xs; ++x) {
                count_set[y][x] = count;
            }
        }

        const auto img = draw_image(count_set);
        auto view = boost::gil::const_view(img);
        ASSERT_EQ(n_xs, view.width());
        ASSERT_EQ(n_ys, view.height());

        const auto pixel = *(view.row_begin(0));
        EXPECT_EQ(HIGH_COLOR_R, boost::gil::at_c<0>(pixel));
        EXPECT_EQ(HIGH_COLOR_G, boost::gil::at_c<1>(pixel));
        EXPECT_EQ(HIGH_COLOR_B, boost::gil::at_c<2>(pixel));
    }
}

TEST_F(TestDrawImage, TwoColors) {
    CountSet count_set(boost::extents[2][3]);
    count_set[1][0] = 1;
    auto n_ys = count_set.shape()[0];
    auto n_xs = count_set.shape()[1];

    const auto img = draw_image(count_set);
    auto view = boost::gil::const_view(img);
    ASSERT_EQ(n_xs, view.width());
    ASSERT_EQ(n_ys, view.height());

    for (decltype(n_ys) y{0}; y < n_ys; ++y) {
        auto it = view.row_begin(y);
        for (decltype(n_xs) x{0}; x < n_xs; ++x, ++it) {
            const auto pixel = *it;
            if ((x == 0) && (y == 1)) {
                EXPECT_EQ(HIGH_COLOR_R, boost::gil::at_c<0>(pixel));
                EXPECT_EQ(HIGH_COLOR_G, boost::gil::at_c<1>(pixel));
                EXPECT_EQ(HIGH_COLOR_B, boost::gil::at_c<2>(pixel));
            } else {
                EXPECT_EQ(LOW_COLOR_R, boost::gil::at_c<0>(pixel));
                EXPECT_EQ(LOW_COLOR_G, boost::gil::at_c<1>(pixel));
                EXPECT_EQ(LOW_COLOR_B, boost::gil::at_c<2>(pixel));
            }
        }
    }
}

TEST_F(TestDrawImage, ManyColors) {
    CountSet count_set(boost::extents[2][3]);
    auto n_ys = count_set.shape()[0];
    auto n_xs = count_set.shape()[1];

    Count count = 0;
    for (decltype(n_ys) y{0}; y < n_ys; ++y) {
        for (decltype(n_xs) x{0}; x < n_xs; ++x) {
            count_set[y][x] = count;
            count = (count == 0) ? 4 : (count * 2);
        }
    }

    const auto img = draw_image(count_set);
    auto view = boost::gil::const_view(img);
    ASSERT_EQ(n_xs, view.width());
    ASSERT_EQ(n_ys, view.height());

    ColorElement prev = 0;
    for (decltype(n_ys) y{0}; y < n_ys; ++y) {
        auto it = view.row_begin(y);
        for (decltype(n_xs) x{0}; x < n_xs; ++x, ++it) {
            const auto pixel = *it;
            const auto current = boost::gil::at_c<0>(pixel);
            if ((x == 0) && (y == 0)) {
                EXPECT_FALSE(boost::gil::at_c<0>(pixel));
            } else {
                EXPECT_LT(prev, current);
            }
            prev = current;
        }
    }
}

class TestWriteCsv : public ::testing::Test {};

TEST_F(TestWriteCsv, Success) {
    const TempFile csv(".csv");
    const auto& csv_filepath = csv.Get();
    ASSERT_TRUE(csv_filepath.has_value());

    CountSet count_set(boost::extents[2][3]);
    auto n_ys = count_set.shape()[0];
    auto n_xs = count_set.shape()[1];

    Count count = 0;
    for (decltype(n_ys) y{0}; y < n_ys; ++y) {
        for (decltype(n_xs) x{0}; x < n_xs; ++x) {
            count_set[y][x] = count;
            count = (count == 0) ? 4 : (count * 2);
        }
    }

    ASSERT_EQ(ExitStatus::SUCCESS, write_csv(count_set, *csv_filepath));
    std::ifstream ifs(*csv_filepath);
    const std::string actual(std::istreambuf_iterator<char>(ifs), {});
    const std::string expected("0,4,8\n16,32,64\n");
    EXPECT_EQ(expected, actual);
}

TEST_F(TestWriteCsv, Failed) {
    const std::filesystem::path empty_filename;
    CountSet count_set(boost::extents[2][3]);
    ASSERT_EQ(ExitStatus::FILE_ERROR, write_csv(count_set, empty_filename));
}

class TestDraw : public ::testing::Test {};

TEST_F(TestDraw, Success) {
    const TempFile csv(".csv");
    const auto& csv_filepath = csv.Get();
    ASSERT_TRUE(csv_filepath.has_value());

    const TempFile png(".png");
    const auto& png_filepath = png.Get();
    ASSERT_TRUE(png_filepath.has_value());

    constexpr PixelSize n_pixels = 16;
    const ParamSet params(0.5, 0.125, 20, n_pixels, csv_filepath, png_filepath);
    ASSERT_EQ(ExitStatus::SUCCESS, draw(params));

    std::ifstream ifs(*csv_filepath);
    std::string line;
    PixelSize n_lines = 0;
    while (std::getline(ifs, line)) {
        std::istringstream iss(line);
        EXPECT_EQ(0, line.find("0,"));
        ++n_lines;
    }
    EXPECT_EQ(n_pixels, n_lines);

    Bitmap img;
    boost::gil::read_image(png_filepath.value().string(), img, boost::gil::png_tag());
    auto view = boost::gil::const_view(img);
    ASSERT_EQ(n_pixels, view.width());
    ASSERT_EQ(n_pixels, view.height());

    const auto pixel = *view.row_begin(0);
    EXPECT_EQ(LOW_COLOR_R, boost::gil::at_c<0>(pixel));
    EXPECT_EQ(LOW_COLOR_G, boost::gil::at_c<1>(pixel));
    EXPECT_EQ(LOW_COLOR_B, boost::gil::at_c<2>(pixel));
}

TEST_F(TestDraw, NoWrites) {
    const std::optional<std::filesystem::path> csv_filepath;
    const std::optional<std::filesystem::path> png_filepath;
    const ParamSet params(0.5, 0.125, 20, 32, csv_filepath, png_filepath);
    ASSERT_EQ(ExitStatus::SUCCESS, draw(params));
}

TEST_F(TestDraw, CsvError) {
    TempFile csv(".csv");
    const auto csv_filepath = csv.Get();
    ASSERT_TRUE(csv_filepath.has_value());
    csv.CauseError();

    TempFile png(".png");
    const auto png_filepath = png.Get();
    ASSERT_TRUE(png_filepath.has_value());

    const ParamSet params(0.5, 0.125, 20, 32, csv_filepath, png_filepath);
    ASSERT_EQ(ExitStatus::FILE_ERROR, draw(params));
}

TEST_F(TestDraw, ImageError) {
    TempFile csv(".csv");
    const auto csv_filepath = csv.Get();
    ASSERT_TRUE(csv_filepath.has_value());

    TempFile png(".png");
    const auto png_filepath = png.Get();
    ASSERT_TRUE(png_filepath.has_value());
    png.CauseError();

    const ParamSet params(0.5, 0.125, 20, 32, csv_filepath, png_filepath);
    ASSERT_EQ(ExitStatus::FILE_ERROR, draw(params));
}

TEST_F(TestDraw, BadCsvFilename) {
    const std::filesystem::path csv_filepath {".."};
    const TempFile png(".png");
    const auto& png_filepath = png.Get();
    ASSERT_TRUE(png_filepath.has_value());

    const ParamSet params(0.5, 0.125, 20, 32, csv_filepath, png_filepath);
    ASSERT_EQ(ExitStatus::FILE_ERROR, draw(params));
}

TEST_F(TestDraw, BadImageFilename) {
    const TempFile csv(".csv");
    const auto& csv_filepath = csv.Get();
    ASSERT_TRUE(csv_filepath.has_value());
    const std::filesystem::path png_filepath {".."};

    const ParamSet params(0.5, 0.125, 20, 32, csv_filepath, png_filepath);
    ASSERT_EQ(ExitStatus::FILE_ERROR, draw(params));
}

class TestParseArgs : public ::testing::Test {};

TEST_F(TestParseArgs, DefaultArgs) {
    const std::vector<std::string> arg_set {"command"};
    const auto [argc, argv] = make_argc_argv(arg_set);
    ASSERT_EQ(1, argc);
    ASSERT_EQ(2, argv.size());
    ASSERT_TRUE(argv.at(0));
    ASSERT_FALSE(argv.at(1));

    const auto actual = parse_args(argc, argv.data());
    std::filesystem::path expected {ParamSet::default_image_filename};

    EXPECT_EQ(ParamSet::default_x_offset, actual.x_offset);
    EXPECT_EQ(ParamSet::default_y_offset, actual.y_offset);
    EXPECT_EQ(ParamSet::default_max_iter, actual.max_iter);
    EXPECT_EQ(ParamSet::default_n_pixels, actual.n_pixels);
    ASSERT_FALSE(actual.csv_filepath.has_value());
    ASSERT_TRUE(actual.image_filepath.has_value());
    EXPECT_EQ(expected, actual.image_filepath.value());
}

TEST_F(TestParseArgs, Long) {
    const std::vector<std::string> arg_set {
        "command",
        "--x_offset", "0.12",
        "--y_offset", "0.34",
        "--max_iter", "56",
        "--size", "78",
        "--csv", "_test.csv",
        "--image", "_test.png"
    };

    const auto [argc, argv] = make_argc_argv(arg_set);
    ASSERT_EQ(arg_set.size(), argc);
    ASSERT_EQ(argc + 1, argv.size());
    ASSERT_FALSE(argv.at(argc));

    const auto actual = parse_args(argc, argv.data());
    std::filesystem::path expected {ParamSet::default_image_filename};

    std::filesystem::path expected_csv {"_test.csv"};
    std::filesystem::path expected_image {"_test.png"};

    EXPECT_FLOAT_EQ(0.12f, actual.x_offset);
    EXPECT_FLOAT_EQ(0.34f, actual.y_offset);
    EXPECT_EQ(56, actual.max_iter);
    EXPECT_EQ(78, actual.n_pixels);
    ASSERT_TRUE(actual.csv_filepath.has_value());
    EXPECT_EQ(expected_csv, actual.csv_filepath.value());
    ASSERT_TRUE(actual.image_filepath.has_value());
    EXPECT_EQ(expected_image, actual.image_filepath.value());
}

TEST_F(TestParseArgs, Short) {
    const std::vector<std::string> arg_set {
        "command",
        "-x", "0.056",
        "-y", "0.078",
        "-m", "21",
        "-s", "43",
        "-c", "_short.csv",
        "-o", "_short.png"
    };

    const auto [argc, argv] = make_argc_argv(arg_set);
    const auto actual = parse_args(argc, argv.data());
    std::filesystem::path expected {ParamSet::default_image_filename};

    std::filesystem::path expected_csv {"_short.csv"};
    std::filesystem::path expected_image {"_short.png"};

    EXPECT_FLOAT_EQ(0.056f, actual.x_offset);
    EXPECT_FLOAT_EQ(0.078f, actual.y_offset);
    EXPECT_EQ(21, actual.max_iter);
    EXPECT_EQ(43, actual.n_pixels);
    ASSERT_TRUE(actual.csv_filepath.has_value());
    EXPECT_EQ(expected_csv, actual.csv_filepath.value());
    ASSERT_TRUE(actual.image_filepath.has_value());
    EXPECT_EQ(expected_image, actual.image_filepath.value());
}

class TestCheckedCast : public ::testing::Test {};

TEST_F(TestCheckedCast, Numbers) {
    using IntType = int;
    using FloatType = float;
    constexpr IntType expected_int = 2;
    constexpr FloatType expected_float = 2;

    IntType from_int = 2;
    double from_double = 2.0;
    double from_double_frac = 2.125;
    float from_float = 2.0;
    float from_float_frac = 2.125f;

    EXPECT_EQ(expected_int, checked_cast<IntType>(from_int));
    EXPECT_EQ(expected_int, checked_cast<IntType>(from_double));
    EXPECT_EQ(expected_int, checked_cast<IntType>(from_double_frac));
    EXPECT_EQ(expected_int, checked_cast<IntType>(from_float));
    EXPECT_EQ(expected_int, checked_cast<IntType>(from_float_frac));

    EXPECT_FLOAT_EQ(expected_float, checked_cast<FloatType>(from_int));
    EXPECT_FLOAT_EQ(expected_float, checked_cast<FloatType>(from_float));
    EXPECT_FLOAT_EQ(expected_float, checked_cast<FloatType>(from_double));
}

TEST_F(TestCheckedCast, ConstRef) {
    using IntType = int;
    constexpr IntType expected = 2;

    double from = 2.125;
    double from_ref = from;
    const double from_const = 2.25;
    const double& from_const_ref = from_const;

    EXPECT_EQ(expected, checked_cast<IntType>(from));
    EXPECT_EQ(expected, checked_cast<IntType>(from_ref));
    EXPECT_EQ(expected, checked_cast<IntType>(from_const));
    EXPECT_EQ(expected, checked_cast<IntType>(from_const_ref));
    EXPECT_EQ(expected, checked_cast<IntType>(2.125));
    EXPECT_EQ(expected, checked_cast<IntType>(from_const - 0.125));

    double will_be_moved = 2.125;
    EXPECT_EQ(expected, checked_cast<IntType>(std::move(will_be_moved)));
}

#if __cplusplus >= 202002L
TEST_F(TestCheckedCast, Cpp20) {
    using IntType = int;
    using FloatType = float;
    constexpr IntType expected_int = 2;
    constexpr FloatType expected_float = 2;
    IntType from_int = 2;
    float from_float = 2.125f;

    EXPECT_EQ(expected_int, checked_cast_cpp20<IntType>(from_float));
    EXPECT_FLOAT_EQ(expected_float, checked_cast_cpp20<FloatType>(from_int));
}
#endif // C++20

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
