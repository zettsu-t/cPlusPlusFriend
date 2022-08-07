#include "juliaset.h"
#include <gtest/gtest.h>

namespace {
using CoordinateSetRef = boost::const_multi_array_ref<Coordinate, 1>;
using CountVector = std::vector<Count>;

void copy_array(const CountVector& src,
                CountSet& dst, PixelSize y, PixelSize n_pixels) {
    boost::const_multi_array_ref<Count, 1> row {src.data(), boost::extents[src.size()]};
    typename CountSet::array_view<1>::type y_view = dst[boost::indices[y][CountSet::index_range(0, n_pixels)]];
    y_view = row;
    return;
}
}

class TestParamSet : public ::testing::Test {};

TEST_F(TestParamSet, Full) {
    constexpr Coordinate x_offset = 0.5;
    constexpr Coordinate y_offset = 0.125;
    constexpr Count max_iter = 20;
    constexpr PixelSize n_pixels = 16;
    const std::string csv_filename {"input.csv"};
    const std::string image_filename {"input.png"};
    const std::optional<std::filesystem::path> csv_path {"input.csv"};
    std::optional<std::filesystem::path> image_path {"input.png"};

    const auto actual = ParamSet(x_offset, y_offset, max_iter, n_pixels,
                                 csv_filename, image_filename);

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
    const auto actual = ParamSet(x_offset, y_offset, max_iter, n_pixels,
                                 csv_filename, image_filename);

    EXPECT_FLOAT_EQ(x_offset, actual.x_offset);
    EXPECT_FLOAT_EQ(y_offset, actual.y_offset);
    EXPECT_EQ(max_iter, actual.max_iter);
    EXPECT_EQ(n_pixels, actual.n_pixels);
    EXPECT_FALSE(actual.csv_filepath.has_value());
    EXPECT_FALSE(actual.image_filepath.has_value());
}

class TestComplex : public ::testing::Test {};

TEST_F(TestComplex, NormSqr) {
    const Point left_top {3.0, 4.0};
    EXPECT_FLOAT_EQ(25.0, norm2_sqr(left_top));

    const Point right_bottom {-2.0, -3.0};
    EXPECT_FLOAT_EQ(13.0, norm2_sqr(right_bottom));
}

class TestTransformPoint : public ::testing::Test {};

TEST_F(TestTransformPoint, All) {
    constexpr Point from {2.0, 4.0};
    constexpr Point offset {3.0, 5.0};
    const auto actual = transform_point(from, offset);
    constexpr Point expected {-9.0, 21.0};

    EXPECT_FLOAT_EQ(expected.real(), actual.real());
    EXPECT_FLOAT_EQ(expected.imag(), actual.imag());
}

class TestConvergePoint : public ::testing::Test {};

TEST_F(TestConvergePoint, All) {
    constexpr Coordinate eps = std::numeric_limits<Coordinate>::epsilon();
    constexpr Coordinate delta = 1e-7f;

    const Point zero {0.0, 0.0};
    const auto actual_zero = converge_point(23.0, 0.0, zero, 100, eps);
    EXPECT_EQ(0, actual_zero);

    const auto actual_real = converge_point(1 + delta, 0.0, zero, 100, eps);
    EXPECT_EQ(22, actual_real);

    const auto actual_imag = converge_point(0.0, 1 + delta, zero, 11, eps);
    EXPECT_EQ(11, actual_imag);

    const Point offset_a {0.5, 0.375};
    const auto actual_offset_a = converge_point(0.0, 0.0, offset_a, 100, eps);
    EXPECT_EQ(4, actual_offset_a);

    const Point offset_b {0.375, 0.5};
    const auto actual_offset_b = converge_point(0.0, 0.0, offset_b, 100, eps);
    EXPECT_EQ(8, actual_offset_b);

    const auto actual_all_a = converge_point(0.5, 0.375, offset_b, 100, eps);
    EXPECT_EQ(3, actual_all_a);

    const auto actual_all_b = converge_point(0.375, 0.5, offset_a, 100, eps);
    EXPECT_EQ(7, actual_all_b);

    const Point offset_c {0.375, 0.375};
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
    const Point offset {0.375, 0.375};
    auto view_xs = xs[boost::indices[decltype(xs)::index_range(0, xs.shape()[0])]];
    auto view_ys = ys[boost::indices[decltype(ys)::index_range(0, ys.shape()[0])]];
    auto actual = converge_point_set(view_xs, view_ys, offset, 100, 1e-5f);
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
    const Point offset {0.375, 0.375};
    auto view_xs = xs[boost::indices[decltype(xs)::index_range(0, xs.shape()[0])]];
    auto view_ys = ys[boost::indices[decltype(ys)::index_range(0, ys.shape()[0])]];
    auto actual = converge_point_set(view_xs, view_ys, offset, 100, 1e-5f);
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
    const Point offset {0.5, 0.375};
    auto view_xs = xs[boost::indices[decltype(xs)::index_range(0, xs.shape()[0])]];
    auto view_ys = ys[boost::indices[decltype(ys)::index_range(0, ys.shape()[0])]];
    auto actual = converge_point_set(view_xs, view_ys, offset, 5, 1e-5f);
    ASSERT_EQ(1, actual.shape()[0]);
    ASSERT_EQ(2, actual.shape()[1]);
    EXPECT_EQ(5, actual[0][0]);
    EXPECT_EQ(3, actual[0][1]);
}

class TestMapCoordinates : public ::testing::Test {};

TEST_F(TestMapCoordinates, Zero) {
    constexpr Coordinate half_length = 1.0f;
    constexpr PixelSize n_pixels = 0;
    const auto actual = map_coordinates(half_length, n_pixels);
    ASSERT_EQ(n_pixels, actual.shape()[0]);
}

TEST_F(TestMapCoordinates, One) {
    constexpr Coordinate half_length = 0.0f;
    constexpr PixelSize n_pixels = 1;
    const auto actual = map_coordinates(half_length, n_pixels);
    ASSERT_EQ(n_pixels, actual.shape()[0]);
    EXPECT_FLOAT_EQ(-half_length, actual[0]);
}

TEST_F(TestMapCoordinates, Two) {
    constexpr Coordinate half_length = 3.0f;
    constexpr PixelSize n_pixels = 2;
    const auto actual = map_coordinates(half_length, n_pixels);
    ASSERT_EQ(n_pixels, actual.shape()[0]);

    std::vector<CoordinateSet::value_type> expected_vec {-3.0, 3.0};
    CoordinateSetRef expected {expected_vec.data(), boost::extents[expected_vec.size()]};
    ASSERT_EQ(expected, actual);
}

TEST_F(TestMapCoordinates, Many) {
    constexpr Coordinate half_length_odd = 2;
    constexpr PixelSize n_pixels_odd = 5;
    const auto actual_odd = map_coordinates(half_length_odd, n_pixels_odd);
    std::vector<CoordinateSet::value_type> expected_odd_vec {-2.0, -1.0, 0.0, 1.0, 2.0};
    CoordinateSetRef expected_odd {expected_odd_vec.data(), boost::extents[expected_odd_vec.size()]};
    ASSERT_EQ(expected_odd, actual_odd);

    constexpr Coordinate half_length_even = 6.0;
    constexpr PixelSize n_pixels_even = 4;
    const auto actual_even = map_coordinates(half_length_even, n_pixels_even);
    std::vector<CoordinateSet::value_type> expected_even_vec {-6.0, -2.0, 2.0, 6.0};
    CoordinateSetRef expected_even {expected_even_vec.data(), boost::extents[expected_even_vec.size()]};
    ASSERT_EQ(expected_even, actual_even);
}

class TestScanPoints : public ::testing::Test {};

TEST_F(TestScanPoints, Capped) {
    constexpr PixelSize n_pixels = 4;
    const auto actual = scan_points(0.25, 0.75, 3, n_pixels);

    CountVector expected_0 {0, 0, 1, 0};
    CountVector expected_1 {0, 2, 3, 0};
    CountVector expected_2 {0, 3, 2, 0};
    CountVector expected_3 {0, 1, 0, 0};

    CountSet expected(boost::extents[n_pixels][n_pixels]);
    copy_array(expected_0, expected, 0, n_pixels);
    copy_array(expected_1, expected, 1, n_pixels);
    copy_array(expected_2, expected, 2, n_pixels);
    copy_array(expected_3, expected, 3, n_pixels);
    EXPECT_EQ(actual, expected);
}

TEST_F(TestScanPoints, Unlimited) {
    constexpr PixelSize n_pixels = 4;
    const auto actual = scan_points(0.25, 0.75, 100, n_pixels);

    CountVector expected_0 {0, 0, 1, 0};
    CountVector expected_1 {0, 2, 5, 0};
    CountVector expected_2 {0, 5, 2, 0};
    CountVector expected_3 {0, 1, 0, 0};

    CountSet expected(boost::extents[n_pixels][n_pixels]);
    copy_array(expected_0, expected, 0, n_pixels);
    copy_array(expected_1, expected, 1, n_pixels);
    copy_array(expected_2, expected, 2, n_pixels);
    copy_array(expected_3, expected, 3, n_pixels);
    EXPECT_EQ(actual, expected);
}

class TestMakeGradientColors : public ::testing::Test {};

TEST_F(TestMakeGradientColors, Zero) {
    const auto table = make_gradient_colors(0);
    ASSERT_EQ(1, table.size());
}

TEST_F(TestMakeGradientColors, One) {
    const auto table = make_gradient_colors(0);
    ASSERT_EQ(1, table.size());
    const auto pixel = table.at(0);
    EXPECT_EQ(LOW_COLOR_R, boost::gil::at_c<0>(pixel));
    EXPECT_EQ(LOW_COLOR_G, boost::gil::at_c<1>(pixel));
    EXPECT_EQ(LOW_COLOR_B, boost::gil::at_c<2>(pixel));
}

TEST_F(TestMakeGradientColors, Three) {
    const auto table = make_gradient_colors(2);
    ASSERT_EQ(3, table.size());

    const auto pixel_low = table.at(0);
    EXPECT_EQ(LOW_COLOR_R, boost::gil::at_c<0>(pixel_low));
    EXPECT_EQ(LOW_COLOR_G, boost::gil::at_c<1>(pixel_low));
    EXPECT_EQ(LOW_COLOR_B, boost::gil::at_c<2>(pixel_low));

    const auto pixel_high = table.at(2);
    EXPECT_EQ(HIGH_COLOR_R, boost::gil::at_c<0>(pixel_high));
    EXPECT_EQ(HIGH_COLOR_G, boost::gil::at_c<1>(pixel_high));
    EXPECT_EQ(HIGH_COLOR_B, boost::gil::at_c<2>(pixel_high));

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

class TestDrawImage : public ::testing::Test {};

TEST_F(TestDrawImage, Monotone) {
    CountSet count_set(boost::extents[2][3]);
    auto n_ys = count_set.shape()[0];
    auto n_xs = count_set.shape()[1];

    for(decltype(n_ys) y {0}; y < n_ys; ++y) {
        for(decltype(n_xs) x {0}; x < n_xs; ++x) {
            count_set[y][x] = 1;
        }
    }

    const auto img = draw_image(count_set);
    auto view = boost::gil::const_view(img);
    ASSERT_EQ(n_xs, view.width());
    ASSERT_EQ(n_ys, view.height());
    auto pixel = *view.row_begin(0);
    EXPECT_EQ(HIGH_COLOR_R, boost::gil::at_c<0>(pixel));
    EXPECT_EQ(HIGH_COLOR_G, boost::gil::at_c<1>(pixel));
    EXPECT_EQ(HIGH_COLOR_B, boost::gil::at_c<2>(pixel));
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

    for(decltype(n_ys) y {0}; y < n_ys; ++y) {
        auto it = view.row_begin(y);
        for(decltype(n_xs) x {0}; x < n_xs; ++x, ++it) {
            auto pixel = *it;
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
    for(decltype(n_ys) y {0}; y < n_ys; ++y) {
        for(decltype(n_xs) x {0}; x < n_xs; ++x) {
            count_set[y][x] = count;
            count = (count == 0) ? 4 : (count * 2);
        }
    }

    const auto img = draw_image(count_set);
    auto view = boost::gil::const_view(img);
    ASSERT_EQ(n_xs, view.width());
    ASSERT_EQ(n_ys, view.height());

    ColorElement prev = 0;
    for(decltype(n_ys) y {0}; y < n_ys; ++y) {
        auto it = view.row_begin(y);
        for(decltype(n_xs) x {0}; x < n_xs; ++x, ++it) {
            auto pixel = *it;
            auto current = boost::gil::at_c<0>(pixel);
            if ((x == 0) && (y == 0)) {
                EXPECT_FALSE(boost::gil::at_c<0>(pixel));
            } else {
                EXPECT_LT(prev, current);
            }
            prev = current;
        }
    }
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
