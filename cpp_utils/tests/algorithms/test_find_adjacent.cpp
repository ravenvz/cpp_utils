#include "cpp_utils/algorithms/alg_ext.h"
#include <gtest/gtest.h>

namespace {

auto differ_by_one = [](int left, int right) {
    return std::abs(right - left) == 1;
};

} // namespace

TEST(FindAdjacentMatches, find_all_adjacent_matches_in_empty_container)
{
    const std::vector<int> values;
    std::vector<std::pair<int, int>> output;

    alg::find_all_adjacent_matches(cbegin(values),
                                   cend(values),
                                   std::back_inserter(output),
                                   differ_by_one);

    EXPECT_TRUE(output.empty());
}

TEST(FindAdjacentMatches, find_all_adjacent_matches_in_singleton_container)
{
    const std::vector<int> values{8};
    std::vector<std::pair<int, int>> output;

    alg::find_all_adjacent_matches(cbegin(values),
                                   cend(values),
                                   std::back_inserter(output),
                                   differ_by_one);

    EXPECT_TRUE(output.empty());
}

TEST(FindAdjacentMatches, find_all_elements_fully_mismatched_contaner)
{
    const std::vector<int> values{2, 4, 6, 8};
    std::vector<std::pair<int, int>> output;

    alg::find_all_adjacent_matches(cbegin(values),
                                   cend(values),
                                   std::back_inserter(output),
                                   differ_by_one);

    EXPECT_TRUE(output.empty());
}

TEST(FindAdjacentMatches, find_all_elements_in_fully_matched_container)
{
    const std::vector<int> values{1, 2, 3, 4};
    const std::vector<std::pair<int, int>> expected{{1, 2}, {2, 3}, {3, 4}};
    std::vector<std::pair<int, int>> output;

    alg::find_all_adjacent_matches(cbegin(values),
                                   cend(values),
                                   std::back_inserter(output),
                                   differ_by_one);

    EXPECT_EQ(expected, output);
}

TEST(FindAdjacentMatches, find_all_elements_in_mixed_container)
{
    const std::vector<int> values{2, 1, 4, 7, 5, 6, 2};
    const std::vector<std::pair<int, int>> expected{{2, 1}, {5, 6}};
    std::vector<std::pair<int, int>> output;

    alg::find_all_adjacent_matches(
        values, std::back_inserter(output), differ_by_one);

    EXPECT_EQ(expected, output);
}

TEST(FindAdjacentMatches, find_all_elements_with_projection)
{
    auto plus7 = [](auto val) { return val + 7; };
    auto divisable_by_5 = [](auto left, auto right) {
        return (left + right) % 5 == 0;
    };
    const std::vector<int> values{2, 1, 4, 7, 5, 6, 2};
    //                    proj   {9, 8, 11, 14, 12, 13, 9};
    //                    pred   {17, 19, 25, 26, 25, 22};
    const std::vector<std::pair<int, int>> expected{{4, 7}, {5, 6}};
    std::vector<std::pair<int, int>> output;

    alg::find_all_adjacent_matches(
        values, std::back_inserter(output), divisable_by_5, plus7);

    EXPECT_EQ(expected, output);
}
