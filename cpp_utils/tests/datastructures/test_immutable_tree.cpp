#include "datastructures/ImmutableTree.h"
#include "gmock/gmock.h"
#include <ranges>

using ::testing::ElementsAre;

using namespace ds;

namespace {

auto make_sample_tree() -> ds::ImmutableTree<int>
{
    /*
     * 1
     *   2
     *     10
     *   3
     * 4
     *   5
     *     6
     *     7
     *       8
     * 9
     */
    std::vector<std::optional<int>> flattened{std::nullopt,
                                              std::nullopt,
                                              1,
                                              4,
                                              9,
                                              std::nullopt,
                                              2,
                                              3,
                                              std::nullopt,
                                              5,
                                              std::nullopt,
                                              std::nullopt,
                                              10,
                                              std::nullopt,
                                              std::nullopt,
                                              6,
                                              7,
                                              std::nullopt,
                                              std::nullopt,
                                              std::nullopt,
                                              8,
                                              std::nullopt,
                                              std::nullopt};
    auto tree = ImmutableTree<int>(std::make_move_iterator(begin(flattened)),
                                   std::make_move_iterator(end(flattened)));
    return tree;
}

} // namespace

class ImmutableTreeFixture : public ::testing::Test {
public:
    ds::ImmutableTree<int> sut{make_sample_tree()};
};

TEST_F(ImmutableTreeFixture, test_iteration)
{
    EXPECT_THAT(sut, ElementsAre(1, 2, 10, 3, 4, 5, 6, 7, 8, 9));
}

TEST_F(ImmutableTreeFixture, transforming_tree)
{
    auto mapped_tree = sut.transform(
        [](auto val) -> std::string { return std::to_string(val); });

    EXPECT_THAT(mapped_tree,
                ElementsAre("1", "2", "10", "3", "4", "5", "6", "7", "8", "9"));
}

TEST_F(ImmutableTreeFixture, following_parents)
{
    auto it = std::ranges::find(sut, 8);
    it = sut.parent(it);
    EXPECT_EQ(7, *it);
    it = sut.parent(it);
    EXPECT_EQ(5, *it);
    it = sut.parent(it);
    EXPECT_EQ(4, *it);
    it = sut.parent(it);
    EXPECT_EQ(sut.cend(), it);
}

TEST_F(ImmutableTreeFixture, provides_view_to_children)
{
    const std::vector<int> expected{6, 7};

    const auto children = sut.children(std::ranges::find(sut, 5));

    EXPECT_TRUE(std::ranges::equal(expected, children));
}

TEST_F(ImmutableTreeFixture, provides_view_to_root_children)
{
    const std::vector<int> expected{1, 4, 9};

    const auto children = sut.children(sut.cend());

    EXPECT_TRUE(std::ranges::equal(expected, children));
}

TEST_F(ImmutableTreeFixture, returns_node_position_in_children)
{
    EXPECT_EQ(1, sut.position_in_children(std::ranges::find(sut, 7)));
    EXPECT_EQ(0, sut.position_in_children(std::ranges::find(sut, 1)));
    EXPECT_EQ(0, sut.position_in_children(sut.cend()));
}
