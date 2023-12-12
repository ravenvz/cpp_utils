#include "cpp_utils/algorithms/optional_ext.h"
#include <gtest/gtest.h>

TEST(MonadicOptionalFixture, test_inspect_optional_does_nothing_when_nullopt)
{
    int x{2};
    auto side_effect = [&x](int y) { x += y; };

    alg::inspect(std::optional<int>{}, side_effect);

    EXPECT_EQ(2, x);
}

TEST(MonadicOptionalFixture,
     test_inspect_calls_side_effect_with_contained_value)
{
    int x{2};
    auto side_effect = [&x](int y) { x += y; };

    alg::inspect(std::optional<int>{5}, side_effect);

    EXPECT_EQ(7, x);
}

TEST(MonadicOptionalFixture, compare_optionals) {
    constexpr auto a_value = std::optional<int>{};
    constexpr auto b_value = std::optional<int>{7};
    constexpr auto c_value = std::optional<int>{90};
    constexpr auto d_value = std::optional<int>{-7};

    // TODO abs will be contexpr in C++23
    constexpr auto cmp_abs = [](const auto& lhs, const auto& rhs) {
        return std::abs(lhs) == std::abs(rhs);
    };

    EXPECT_TRUE(alg::opt_equal(b_value, d_value, cmp_abs));
    EXPECT_TRUE(alg::opt_equal(a_value, a_value, cmp_abs));
    EXPECT_FALSE(alg::opt_equal(a_value, d_value, cmp_abs));
    EXPECT_FALSE(alg::opt_equal(c_value, d_value, cmp_abs));
}
