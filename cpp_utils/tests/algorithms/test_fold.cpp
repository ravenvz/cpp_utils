#include "algorithms/ranges_ext.h"
#include <gtest/gtest.h>

TEST(FoldFixture, fold_empty_sequence)
{
    const std::vector<int> values;
    const auto init = 7;

    const auto actual = alg::fold(values, 7, std::plus<int>{});

    EXPECT_EQ(init, actual);
}

TEST(FoldFixture, fold_sequence)
{
    const std::vector<int> values{1, 2, 3, 4};
    const int expected{10};

    const auto actual = alg::fold(values, 0, std::plus<int>{});

    EXPECT_EQ(expected, actual);
}

TEST(FoldFixture, fold_sequence_with_projection)
{
    const std::vector<int> values{1, 2, 3, 4};
    const int expected{30};

    const auto actual = alg::fold(
        values, 0, std::plus<int>{}, [](auto val) { return val * val; });

    EXPECT_EQ(expected, actual);
}

TEST(FoldFixture, fold_to_different_type)
{
    const std::vector<int> values{1, 2, 3, 4};
    const std::string expected{"1, 2, 3, 4, "};

    const auto actual =
        alg::fold(values, std::string{}, [](const auto& acc, const auto& val) {
            return acc + std::to_string(val) + ", ";
        });

    EXPECT_EQ(expected, actual);
}

TEST(FoldFixture, fold_on_iterators)
{
    const std::vector<int> values{1, 2, 3, 4, 5, 6};
    const int expected{10};

    const auto actual =
        alg::fold(begin(values),
                  begin(values) + 4,
                  0,
                  [](const auto& acc, const auto& val) { return acc + val; });

    EXPECT_EQ(expected, actual);
}
