#include <cmath>
#include <iostream>
#include <random>
#include <vector>
#include <gtest/gtest.h>

// Assumes discrete distributions
using Value = int;
using Probability = double;
using Mean = double;
using ProbabilityDensity = std::vector<Probability>;
constexpr std::mt19937::result_type GlobalSeed = 123;

template <typename T>
ProbabilityDensity MakeDistribution(Value maxNumber, Value trialSize, T& dist) {
    std::mt19937 gen{GlobalSeed};
    ProbabilityDensity prob(maxNumber + 1);
    const Probability unit = 1.0 / trialSize;
    for(auto i = decltype(trialSize){0}; i < trialSize; ++i) {
        Value num = std::round(dist(gen));
        if ((num >= 0) && (num <= maxNumber)) {
            prob.at(num) += unit;
        }
    }

    return prob;
}

ProbabilityDensity MakeNormalDistribution(Mean mean, Mean variance, Value maxNumber, Value trialSize) {
    std::mt19937 gen{GlobalSeed};
    std::normal_distribution<Probability> dist(mean, std::sqrt(variance));
    return MakeDistribution(maxNumber, trialSize, dist);
}

ProbabilityDensity MakePoissonDistribution(Mean mean, Value maxNumber, Value trialSize) {
    std::mt19937 gen{GlobalSeed};
    std::poisson_distribution<Value> dist(mean);
    return MakeDistribution(maxNumber, trialSize, dist);
}

ProbabilityDensity MakeNegativeBinomialDistribution(Value size, Mean prob, Value maxNumber, Value trialSize) {
    std::mt19937 gen{GlobalSeed};
    std::negative_binomial_distribution<Value> dist(size, prob);
    return MakeDistribution(maxNumber, trialSize, dist);
}

// Probability density from 0 to n (n+1 elements)
ProbabilityDensity MergeProbabilityDensity(const ProbabilityDensity& lhs, const ProbabilityDensity& rhs) {
    if (lhs.empty()) {
        return rhs;
    }

    if (rhs.empty()) {
        return lhs;
    }

    const auto maxLeft = lhs.size() - 1;
    const auto maxRight = rhs.size() - 1;
    const auto maxJoint = maxLeft + maxRight;
    ProbabilityDensity jointProb(maxJoint + 1);

    for(auto iRight = decltype(maxRight){0}; iRight <= maxRight; ++iRight) {
        const auto probRight = rhs.at(iRight);
        for(auto iLeft = decltype(maxLeft){0}; iLeft <= maxLeft; ++iLeft) {
            jointProb.at(iLeft + iRight) += lhs.at(iLeft) * probRight;
        }
    }

    return jointProb;
}

struct Moments {
    Mean mean {0};
    Mean variance {0};
};

Moments GetMeanVariance(const ProbabilityDensity& dist, Probability epsilon) {
    Value value = 0;
    Mean actualMean = 0.0;
    for (const auto& prob : dist) {
        actualMean += value * prob;
        ++value;
    }

    Mean actualVariance = 0.0;
    value = 0;
    for (const auto& prob : dist) {
        actualVariance += (value - actualMean) * (value - actualMean) * prob;
        ++value;
    }

    EXPECT_NEAR(1.0, std::accumulate(dist.begin(), dist.end(), 0.0), epsilon);
    Moments result {actualMean, actualVariance};
    return result;
}

class TestMergeProbabilityDensity : public ::testing::Test {};

TEST_F(TestMergeProbabilityDensity, Normal) {
    constexpr Mean expectedMean = 50;
    constexpr Mean expectedVariance = 25;
    constexpr Value maxNumber = 150;
    constexpr Value trialSize = 1000000;

    const auto dist = MakeNormalDistribution(expectedMean, expectedVariance, maxNumber, trialSize);
    const auto actual = GetMeanVariance(dist, 0.001);
    EXPECT_EQ(expectedMean, std::round(actual.mean));
    EXPECT_EQ(std::round(expectedVariance), std::round(actual.variance));
}

TEST_F(TestMergeProbabilityDensity, Poisson) {
    constexpr Mean expectedMean = 10;
    constexpr Mean expectedVariance = expectedMean;
    constexpr Value maxNumber = 100;
    constexpr Value trialSize = 1000000;

    const auto dist = MakePoissonDistribution(expectedMean, maxNumber, trialSize);
    const auto actual = GetMeanVariance(dist, 0.001);
    EXPECT_EQ(expectedMean, std::round(actual.mean));
    EXPECT_EQ(std::round(expectedVariance), std::round(actual.variance));
}

TEST_F(TestMergeProbabilityDensity, NegativeBinomial) {
    constexpr Value size = 10;
    constexpr Mean probToSuccess = 0.4;
    constexpr Mean expectedMean = static_cast<Mean>(size) * (1.0 - probToSuccess) / probToSuccess;
    constexpr Mean expectedVariance = expectedMean / probToSuccess;
    constexpr Value maxNumber = 100;
    constexpr Value trialSize = 1000000;

    const auto dist = MakeNegativeBinomialDistribution(size, probToSuccess, maxNumber, trialSize);
    const auto actual = GetMeanVariance(dist, 0.001);
    EXPECT_EQ(expectedMean, std::round(actual.mean));
    EXPECT_EQ(std::round(expectedVariance), std::round(actual.variance));
}

TEST_F(TestMergeProbabilityDensity, SumNormals) {
    const std::vector<Moments> testCases {{10, 16}, {50, 25}, {60, 36}, {70, 49}};
    const std::vector<Moments>::size_type testCaseSize = testCases.size();
    constexpr Value maxNumber = 150;
    constexpr Value trialSize = 100000;

    for(auto size = decltype(testCaseSize){1}; size <= testCaseSize; ++size) {
        ProbabilityDensity accumDist;
        Mean expectedMean = 0.0;
        Mean expectedVariance = 0.0;
        for(auto i = decltype(size){0}; i < size; ++i) {
            auto mean = testCases.at(i).mean;
            auto variance = testCases.at(i).variance;
            expectedMean += mean;
            expectedVariance += variance;
            const auto dist = MakeNormalDistribution(mean, variance, maxNumber, trialSize);
            accumDist = MergeProbabilityDensity(accumDist, dist);
        }

        const auto actual = GetMeanVariance(accumDist, 0.1);
        EXPECT_TRUE(((expectedMean - 1) < actual.mean) && (actual.mean < (expectedMean + 1)));
        EXPECT_TRUE(((expectedVariance - 1) < actual.variance) && (actual.variance < (expectedVariance + 1)));
    }
}

TEST_F(TestMergeProbabilityDensity, SumPoisson) {
    const std::vector<Mean> testCases {4, 8, 12, 16};
    const std::vector<Moments>::size_type testCaseSize = testCases.size();
    constexpr Value maxNumber = 255;
    constexpr Value trialSize = 100000;

    for(auto size = decltype(testCaseSize){1}; size <= testCaseSize; ++size) {
        ProbabilityDensity accumDist;
        Mean expected = 0.0;
        for(auto i = decltype(size){0}; i < size; ++i) {
            auto mean = testCases.at(i);
            expected += mean;
            const auto dist = MakePoissonDistribution(mean, maxNumber, trialSize);
            accumDist = MergeProbabilityDensity(accumDist, dist);
        }

        const auto actual = GetMeanVariance(accumDist, 0.1);
        EXPECT_TRUE(((expected - 1) < actual.mean) && (actual.mean < (expected + 1)));
        EXPECT_TRUE(((expected - 1) < actual.variance) && (actual.variance < (expected + 1)));
    }
}

TEST_F(TestMergeProbabilityDensity, SumNegativeBinomial) {
    struct Params {
        Value size {0};
        Mean probToSuccess {0.0};
    };

    const std::vector<Params> testCases {{3, 0.8}, {8, 0.6}, {10, 0.45}, {20, 0.7}};
    const std::vector<Params>::size_type testCaseSize = testCases.size();
    constexpr Value maxNumber = 150;
    constexpr Value trialSize = 100000;

    for(auto size = decltype(testCaseSize){1}; size <= testCaseSize; ++size) {
        ProbabilityDensity accumDist;
        Mean expectedMean = 0.0;
        Mean expectedVariance = 0.0;
        for(auto i = decltype(size){0}; i < size; ++i) {
            const auto& testCase = testCases[i];
            auto mean = static_cast<Mean>(testCase.size) * (1.0 - testCase.probToSuccess) / testCase.probToSuccess;
            auto variance = mean / testCase.probToSuccess;
            expectedMean += mean;
            expectedVariance += variance;
            const auto dist = MakeNegativeBinomialDistribution(testCase.size, testCase.probToSuccess, maxNumber, trialSize);
            accumDist = MergeProbabilityDensity(accumDist, dist);
        }

        const auto actual = GetMeanVariance(accumDist, 0.1);
        EXPECT_TRUE(((expectedMean - 1) < actual.mean) && (actual.mean < (expectedMean + 1)));
        EXPECT_TRUE(((expectedVariance - 1) < actual.variance) && (actual.variance < (expectedVariance + 1)));
    }
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
