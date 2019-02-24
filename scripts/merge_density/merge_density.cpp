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

ProbabilityDensity MakeNormalDistribution(Mean mean, Mean variance, Value maxNumber, Value trialSize) {
    std::mt19937 gen{GlobalSeed};
    std::normal_distribution<Probability> dist(mean, std::sqrt(variance));

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

ProbabilityDensity MakePoissonDistribution(Mean mean, Value maxNumber, Value trialSize) {
    std::mt19937 gen{GlobalSeed};
    std::poisson_distribution<Value> dist(mean);

    ProbabilityDensity prob(maxNumber + 1);
    const Probability unit = 1.0 / trialSize;
    for(auto i = decltype(trialSize){0}; i < trialSize; ++i) {
        Value num = dist(gen);
        if ((num >= 0) && (num <= maxNumber)) {
            prob.at(num) += unit;
        }
    }

    return prob;
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

TEST_F(TestMergeProbabilityDensity, SumNormals) {
    const std::vector<Moments> testCases {{10, 16}, {50, 25}, {60, 36}, {70, 49}};
    const std::vector<Moments>::size_type testCaseSize = testCases.size();
    constexpr Value maxNumber = 150;
    constexpr Value trialSize = 1000000;

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
