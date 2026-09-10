#include <cpp_utils/algorithms/alg_ext.hpp>
#include <gtest/gtest.h>

using namespace cpp_utils::algorithm;

namespace {

auto differ_by_one = [](int left, int right) {
    return std::abs(right - left) == 1;
};

} // namespace

TEST(FindAdjacentMatches, find_all_adjacent_matches_in_empty_container)
{
    const std::vector<int> values;
    std::vector<std::pair<int, int>> output;

    find_all_adjacent_matches(cbegin(values),
                              cend(values),
                              std::back_inserter(output),
                              differ_by_one);

    EXPECT_TRUE(output.empty());
}

TEST(FindAdjacentMatches, find_all_adjacent_matches_in_singleton_container)
{
    const std::vector<int> values{8};
    std::vector<std::pair<int, int>> output;

    find_all_adjacent_matches(cbegin(values),
                              cend(values),
                              std::back_inserter(output),
                              differ_by_one);

    EXPECT_TRUE(output.empty());
}

TEST(FindAdjacentMatches, find_all_elements_fully_mismatched_contaner)
{
    const std::vector<int> values{2, 4, 6, 8};
    std::vector<std::pair<int, int>> output;

    find_all_adjacent_matches(cbegin(values),
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

    find_all_adjacent_matches(cbegin(values),
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

    find_all_adjacent_matches(
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

    find_all_adjacent_matches(
        values, std::back_inserter(output), divisable_by_5, plus7);

    EXPECT_EQ(expected, output);
}

class SlideFixture : public ::testing::Test {
public:
    std::vector<int> data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
};

TEST_F(SlideFixture, sliding_one_element_to_one_pos_to_the_right)
{
    std::vector<int> expected{1, 3, 2, 4, 5, 6, 7, 8, 9, 10};

    auto p = slide(data.begin() + 1, data.begin() + 2, data.begin() + 3);

    EXPECT_EQ(data.begin() + 2, p.first);
    EXPECT_EQ(data.begin() + 3, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_single_element_to_the_right)
{
    std::vector<int> expected{1, 3, 4, 2, 5, 6, 7, 8, 9, 10};

    auto p = slide(data.begin() + 1, data.begin() + 2, data.begin() + 4);

    EXPECT_EQ(data.begin() + 3, p.first);
    EXPECT_EQ(data.begin() + 4, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_last_element_to_the_right)
{
    std::vector<int> expected{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto p = slide(data.begin() + 9, data.begin() + 10, data.begin() + 10);

    EXPECT_EQ(data.begin() + 9, p.first);
    EXPECT_EQ(data.begin() + 10, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_first_element_to_the_back)
{
    std::vector<int> expected{2, 3, 4, 5, 6, 7, 8, 9, 10, 1};

    auto p = slide(data.begin(), data.begin() + 1, data.begin() + 10);

    EXPECT_EQ(data.begin() + 9, p.first);
    EXPECT_EQ(data.begin() + 10, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_last_element_to_the_front)
{
    std::vector<int> expected{10, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    auto p = slide(data.begin() + 9, data.begin() + 10, data.begin());

    EXPECT_EQ(data.begin(), p.first);
    EXPECT_EQ(data.begin() + 1, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_single_element_to_the_left)
{
    std::vector<int> expected{1, 9, 2, 3, 4, 5, 6, 7, 8, 10};

    auto p = slide(data.begin() + 8, data.begin() + 9, data.begin() + 1);

    EXPECT_EQ(data.begin() + 1, p.first);
    EXPECT_EQ(data.begin() + 2, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_first_element_to_the_left)
{
    std::vector<int> expected{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto p = slide(data.begin(), data.begin() + 1, data.begin());

    EXPECT_EQ(data.begin(), p.first);
    EXPECT_EQ(data.begin() + 1, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_group_of_elements_to_the_right)
{
    std::vector<int> expected{1, 2, 6, 7, 8, 9, 3, 4, 5, 10};

    auto p = slide(data.begin() + 2, data.begin() + 5, data.begin() + 9);

    EXPECT_EQ(data.begin() + 6, p.first);
    EXPECT_EQ(data.begin() + 9, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_group_of_elements_to_the_left)
{
    std::vector<int> expected{1, 7, 8, 9, 2, 3, 4, 5, 6, 10};

    auto p = slide(data.begin() + 6, data.begin() + 9, data.begin() + 1);

    EXPECT_EQ(data.begin() + 1, p.first);
    EXPECT_EQ(data.begin() + 4, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_segment_inside_itself_does_nothing)
{
    std::vector<int> expected{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto p = slide(data.begin() + 1, data.begin() + 4, data.begin() + 1);

    EXPECT_EQ(data.begin() + 1, p.first);
    EXPECT_EQ(data.begin() + 4, p.second);
    EXPECT_EQ(expected, data);
}

TEST(FoldFixture, fold_empty_sequence)
{
    const std::vector<int> values;
    const auto init = 7;

    const auto actual = fold(values, 7, std::plus<int>{});

    EXPECT_EQ(init, actual);
}

TEST(FoldFixture, fold_sequence)
{
    const std::vector<int> values{1, 2, 3, 4};
    const int expected{10};

    const auto actual = fold(values, 0, std::plus<int>{});

    EXPECT_EQ(expected, actual);
}

TEST(FoldFixture, fold_sequence_with_projection)
{
    const std::vector<int> values{1, 2, 3, 4};
    const int expected{30};

    const auto actual =
        fold(values, 0, std::plus<int>{}, [](auto val) { return val * val; });

    EXPECT_EQ(expected, actual);
}

TEST(FoldFixture, fold_to_different_type)
{
    const std::vector<int> values{1, 2, 3, 4};
    const std::string expected{"1, 2, 3, 4, "};

    const auto actual =
        fold(values, std::string{}, [](const auto& acc, const auto& val) {
            return acc + std::to_string(val) + ", ";
        });

    EXPECT_EQ(expected, actual);
}

TEST(FoldFixture, fold_on_iterators)
{
    const std::vector<int> values{1, 2, 3, 4, 5, 6};
    const int expected{10};

    const auto actual =
        fold(begin(values),
             begin(values) + 4,
             0,
             [](const auto& acc, const auto& val) { return acc + val; });

    EXPECT_EQ(expected, actual);
}
