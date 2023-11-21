#include "algorithms/alg_ext.h"
#include <gtest/gtest.h>

class SlideFixture : public ::testing::Test {
public:
    std::vector<int> data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
};

TEST_F(SlideFixture, sliding_one_element_to_one_pos_to_the_right)
{
    std::vector<int> expected{1, 3, 2, 4, 5, 6, 7, 8, 9, 10};

    auto p = alg::slide(data.begin() + 1, data.begin() + 2, data.begin() + 3);

    EXPECT_EQ(data.begin() + 2, p.first);
    EXPECT_EQ(data.begin() + 3, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_single_element_to_the_right)
{
    std::vector<int> expected{1, 3, 4, 2, 5, 6, 7, 8, 9, 10};

    auto p = alg::slide(data.begin() + 1, data.begin() + 2, data.begin() + 4);

    EXPECT_EQ(data.begin() + 3, p.first);
    EXPECT_EQ(data.begin() + 4, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_last_element_to_the_right)
{
    std::vector<int> expected{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto p = alg::slide(data.begin() + 9, data.begin() + 10, data.begin() + 10);

    EXPECT_EQ(data.begin() + 9, p.first);
    EXPECT_EQ(data.begin() + 10, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_first_element_to_the_back)
{
    std::vector<int> expected{2, 3, 4, 5, 6, 7, 8, 9, 10, 1};

    auto p = alg::slide(data.begin(), data.begin() + 1, data.begin() + 10);

    EXPECT_EQ(data.begin() + 9, p.first);
    EXPECT_EQ(data.begin() + 10, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_last_element_to_the_front)
{
    std::vector<int> expected{10, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    auto p = alg::slide(data.begin() + 9, data.begin() + 10, data.begin());

    EXPECT_EQ(data.begin(), p.first);
    EXPECT_EQ(data.begin() + 1, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_single_element_to_the_left)
{
    std::vector<int> expected{1, 9, 2, 3, 4, 5, 6, 7, 8, 10};

    auto p = alg::slide(data.begin() + 8, data.begin() + 9, data.begin() + 1);

    EXPECT_EQ(data.begin() + 1, p.first);
    EXPECT_EQ(data.begin() + 2, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_first_element_to_the_left)
{
    std::vector<int> expected{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto p = alg::slide(data.begin(), data.begin() + 1, data.begin());

    EXPECT_EQ(data.begin(), p.first);
    EXPECT_EQ(data.begin() + 1, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_group_of_elements_to_the_right)
{
    std::vector<int> expected{1, 2, 6, 7, 8, 9, 3, 4, 5, 10};

    auto p = alg::slide(data.begin() + 2, data.begin() + 5, data.begin() + 9);

    EXPECT_EQ(data.begin() + 6, p.first);
    EXPECT_EQ(data.begin() + 9, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_group_of_elements_to_the_left)
{
    std::vector<int> expected{1, 7, 8, 9, 2, 3, 4, 5, 6, 10};

    auto p = alg::slide(data.begin() + 6, data.begin() + 9, data.begin() + 1);

    EXPECT_EQ(data.begin() + 1, p.first);
    EXPECT_EQ(data.begin() + 4, p.second);
    EXPECT_EQ(expected, data);
}

TEST_F(SlideFixture, sliding_segment_inside_itself_does_nothing)
{
    std::vector<int> expected{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto p = alg::slide(data.begin() + 1, data.begin() + 4, data.begin() + 1);

    EXPECT_EQ(data.begin() + 1, p.first);
    EXPECT_EQ(data.begin() + 4, p.second);
    EXPECT_EQ(expected, data);
}
