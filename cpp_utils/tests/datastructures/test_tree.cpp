#include "datastructures/Tree.h"
#include "gmock/gmock.h"

using namespace ds;

auto make_sample_tree() -> ds::Tree<int>
{
    /*
     * 1
     *   2
     *   3
     *     4
     *     5
     *   12
     *      7
     *        8
     *          9
     *   5
     *     1
     *     2
     */
    ds::Tree<int> tree;

    auto one = tree.insert(tree.end(), 1);
    tree.insert(one, 2);
    auto three = tree.insert(one, 3);
    tree.insert(three, 4);
    tree.insert(three, 5);
    auto twelve = tree.insert(one, 12);
    auto seven = tree.insert(twelve, 7);
    auto eight = tree.insert(seven, 8);
    tree.insert(eight, 9);
    auto five_dup = tree.insert(one, 5);
    tree.insert(five_dup, 1);
    tree.insert(five_dup, 2);

    return tree;
}

auto make_multiroot_sample_tree() -> ds::Tree<int>
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
    ds::Tree<int> tree;

    auto one = tree.insert(tree.end(), 1);
    auto two = tree.insert(one, 2);
    tree.insert(two, 10);
    tree.insert(one, 3);
    auto four = tree.insert(tree.end(), 4);
    auto five = tree.insert(four, 5);
    tree.insert(five, 6);
    auto seven = tree.insert(five, 7);
    tree.insert(seven, 8);
    tree.insert(tree.end(), 9);

    return tree;
}

auto make_move_testing_tree() -> ds::Tree<int>
{
    /*
     *
     * 1
     *   2
     *   3
     *     10
     *        11
     *   4
     *   5
     * 6
     *   7
     *   8
     *   9
     *   12
     */
    ds::Tree<int> tree;
    auto one = tree.insert(tree.end(), 1);
    tree.insert(one, 2);
    auto three = tree.insert(one, 3);
    auto ten = tree.insert(three, 10);
    tree.insert(ten, 11);
    tree.insert(one, 4);
    tree.insert(one, 5);
    auto six = tree.insert(tree.end(), 6);
    tree.insert(six, 7);
    tree.insert(six, 8);
    tree.insert(six, 9);
    tree.insert(six, 12);

    return tree;
}

class TestTreeFixture : public ::testing::Test {
public:
    ds::Tree<int> sut{make_sample_tree()};
};

TEST_F(TestTreeFixture, dsf_iterator_traversal)
{
    std::vector<int> actual;
    std::ranges::copy(sut, std::back_inserter(actual));

    EXPECT_THAT(actual,
                ::testing::ElementsAre(1, 2, 3, 4, 5, 12, 7, 8, 9, 5, 1, 2));
}

TEST_F(TestTreeFixture, dfs_iterator_with_multiroot_tree)
{
    const auto tree = make_multiroot_sample_tree();
    std::vector<int> actual;
    std::ranges::copy(tree, std::back_inserter(actual));

    EXPECT_THAT(actual, ::testing::ElementsAre(1, 2, 10, 3, 4, 5, 6, 7, 8, 9));
}

TEST_F(TestTreeFixture, test_mutating_dfs_iterator)
{
    std::ranges::for_each(sut, [](auto& elem) { elem *= 2; });
    std::vector<int> dfs_order;
    std::ranges::copy(sut, std::back_inserter(dfs_order));

    EXPECT_THAT(
        dfs_order,
        ::testing::ElementsAre(2, 4, 6, 8, 10, 24, 14, 16, 18, 10, 2, 4));
}

TEST_F(TestTreeFixture, inserting_children_at_different_positions)
{
    /*
     * 77
     *    6
     *    7
     *      3
     *      4
     *      9
     *        1
     *        12
     *    8
     */

    // Test insertions at different positions
    ds::Tree<int> tree;
    auto root = tree.insert(tree.end(), 77);
    tree.insert(root, 8);
    tree.insert(root, 6, DestinationPosition{0});
    auto seven = tree.insert(root, 7, DestinationPosition{1});
    auto nine = tree.insert(seven, 9);
    tree.insert(seven, 3, DestinationPosition{0});
    tree.insert(seven, 4, DestinationPosition{1});
    tree.insert(nine, 1, DestinationPosition{0});
    tree.insert(
        nine, 12, DestinationPosition{200}); // Given position is beyond range

    std::vector<int> actual_traversal;
    std::ranges::copy(tree, std::back_inserter(actual_traversal));

    EXPECT_THAT(actual_traversal,
                ::testing::ElementsAre(77, 6, 7, 3, 4, 9, 1, 12, 8));
}

TEST_F(TestTreeFixture, test_convertion_of_iterator_to_const_iterator)
{
    auto it = std::ranges::find(sut, 12);

    std::vector<int> traversal;
    std::ranges::copy(it, sut.cend(), std::back_inserter(traversal));

    EXPECT_THAT(traversal, ::testing::ElementsAre(12, 7, 8, 9, 5, 1, 2));
}

TEST_F(TestTreeFixture, tranforming_tree)
{
    auto mapped = sut.transform(
        [](const auto& x) -> std::string { return std::to_string(x); });

    std::vector<std::string> traversal;
    std::ranges::copy(mapped, std::back_inserter(traversal));

    EXPECT_THAT(
        traversal,
        ::testing::ElementsAre(
            "1", "2", "3", "4", "5", "12", "7", "8", "9", "5", "1", "2"));
}

TEST_F(TestTreeFixture, transforming_multiroot_tree)
{
    const auto tree = make_multiroot_sample_tree();
    const auto mapped =
        tree.transform([](const auto& x) { return std::to_string(x); });

    std::vector<std::string> traversal;
    std::ranges::copy(mapped, std::back_inserter(traversal));

    EXPECT_THAT(traversal,
                ::testing::ElementsAre(
                    "1", "2", "10", "3", "4", "5", "6", "7", "8", "9"));
}

TEST_F(TestTreeFixture, test_deep_copy)
{
    ds::Tree<int> tree{sut};

    EXPECT_EQ(sut, tree);
}

TEST_F(TestTreeFixture, copy_assignment)
{
    ds::Tree<int> tree;

    tree = sut;

    EXPECT_EQ(sut, tree);
}

TEST_F(TestTreeFixture, flatten)
{
    const auto tree = make_multiroot_sample_tree();
    EXPECT_THAT(tree.flatten(),
                ::testing::ElementsAre(std::nullopt,
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
                                       std::nullopt));
}

TEST_F(TestTreeFixture, unflatted)
{
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
    const auto restored = ds::Tree<int>::unflatten(flattened);
    const auto tree = make_multiroot_sample_tree();

    EXPECT_EQ(tree, restored);
}

TEST_F(TestTreeFixture, flatten_and_unflatten)
{
    const auto flattened = sut.flatten();

    const auto restored = ds::Tree<int>::unflatten(flattened);

    EXPECT_EQ(sut, restored);
}

TEST_F(TestTreeFixture, take_subtree)
{
    auto tree = make_multiroot_sample_tree();
    auto it = std::ranges::find(tree, 5);

    const auto subtree = tree.take_subtree(it);

    EXPECT_THAT(tree, ::testing::ElementsAre(1, 2, 10, 3, 4, 9));
    EXPECT_THAT(subtree, ::testing::ElementsAre(5, 6, 7, 8));
}

TEST_F(TestTreeFixture, erase_subtree)
{
    auto tree = make_multiroot_sample_tree();
    auto it = std::ranges::find(tree, 4);

    tree.erase(it);

    EXPECT_THAT(tree, ::testing::ElementsAre(1, 2, 10, 3, 9));
}

TEST_F(TestTreeFixture, move_subtree_to_itself)
{
    auto tree = make_multiroot_sample_tree();
    auto source_parent = std::ranges::find(tree, 2);
    auto destination_parent = source_parent;

    tree.move_nodes(source_parent,
                    SourcePosition{0},
                    Count{1},
                    destination_parent,
                    DestinationPosition{0});

    EXPECT_EQ(make_multiroot_sample_tree(), tree);
}

TEST_F(TestTreeFixture, moving_nodes_between_leaves)
{
    auto tree = make_multiroot_sample_tree();
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

    /*
     * 1
     *   3
     * 4
     *   5
     *     6
     *     7
     *       8
     * 9
     *   2
     *     10
     */
    auto source_it = std::ranges::find(tree, 1);
    auto destination_parent = std::ranges::find(tree, 9);
    tree.move_nodes(source_it,
                    SourcePosition{0},
                    Count{1},
                    destination_parent,
                    DestinationPosition{0});

    EXPECT_THAT(tree, ::testing::ElementsAre(1, 3, 4, 5, 6, 7, 8, 9, 2, 10));
}

TEST_F(TestTreeFixture, moving_nodes_up_within_same_parent)
{
    /*
     * 1
     * 2
     * 3
     * 4
     */

    /*
     * 4
     * 1
     * 2
     * 3
     */
    ds::Tree<int> tree;
    tree.insert(tree.end(), 1);
    tree.insert(tree.end(), 2);
    tree.insert(tree.end(), 3);
    tree.insert(tree.end(), 4);
    ds::Tree<int> expected;
    expected.insert(expected.end(), 4);
    expected.insert(expected.end(), 1);
    expected.insert(expected.end(), 2);
    expected.insert(expected.end(), 3);

    tree.move_nodes(tree.end(),
                    SourcePosition{3},
                    Count{1},
                    tree.end(),
                    DestinationPosition{0});

    EXPECT_EQ(expected, tree);
}

TEST_F(TestTreeFixture, moving_nodes_down_within_same_parent)
{
    /*
     * 1
     * 2
     * 3
     * 4
     */

    /*
     * 2
     * 3
     * 1
     * 4
     */
    ds::Tree<int> tree;
    tree.insert(tree.end(), 1);
    tree.insert(tree.end(), 2);
    tree.insert(tree.end(), 3);
    tree.insert(tree.end(), 4);
    ds::Tree<int> expected;
    expected.insert(expected.end(), 2);
    expected.insert(expected.end(), 3);
    expected.insert(expected.end(), 1);
    expected.insert(expected.end(), 4);

    tree.move_nodes(tree.end(),
                    SourcePosition{0},
                    Count{1},
                    tree.end(),
                    DestinationPosition{3});

    EXPECT_THAT(tree, ::testing::ElementsAre(2, 3, 1, 4));
    EXPECT_EQ(expected, tree);
}

TEST_F(TestTreeFixture,
       moving_multiple_nodes_within_the_same_parent_from_left_to_mid)
{
    /*
     * 1
     * 2
     * 3
     * 4
     */

    /*
     * 3
     * 1
     * 2
     * 4
     */

    ds::Tree<int> tree;
    tree.insert(tree.end(), 1);
    tree.insert(tree.end(), 2);
    tree.insert(tree.end(), 3);
    tree.insert(tree.end(), 4);
    ds::Tree<int> expected;
    expected.insert(expected.end(), 3);
    expected.insert(expected.end(), 1);
    expected.insert(expected.end(), 2);
    expected.insert(expected.end(), 4);

    tree.move_nodes(tree.end(),
                    SourcePosition{0},
                    Count{2},
                    tree.end(),
                    DestinationPosition{3});

    EXPECT_EQ(expected, tree);
}

TEST_F(TestTreeFixture,
       moving_multiple_nodes_within_the_same_parent_from_mid_to_end)
{
    /*
     * 1
     * 2
     * 3
     * 4
     */

    /*
     * 1
     * 4
     * 2
     * 3
     */

    ds::Tree<int> tree;
    tree.insert(tree.end(), 1);
    tree.insert(tree.end(), 2);
    tree.insert(tree.end(), 3);
    tree.insert(tree.end(), 4);
    ds::Tree<int> expected;
    expected.insert(expected.end(), 1);
    expected.insert(expected.end(), 4);
    expected.insert(expected.end(), 2);
    expected.insert(expected.end(), 3);

    tree.move_nodes(tree.end(),
                    SourcePosition{1},
                    Count{2},
                    tree.end(),
                    DestinationPosition{4});

    EXPECT_THAT(tree, testing::ElementsAre(1, 4, 2, 3));
    EXPECT_EQ(expected, tree);
}

TEST_F(TestTreeFixture,
       moving_multiple_nodes_within_the_same_parent_from_right_to_begin)
{
    /*
     * 1
     * 2
     * 3
     * 4
     */

    /*
     * 3
     * 4
     * 1
     * 2
     */

    ds::Tree<int> tree;
    tree.insert(tree.end(), 1);
    tree.insert(tree.end(), 2);
    tree.insert(tree.end(), 3);
    tree.insert(tree.end(), 4);
    ds::Tree<int> expected;
    expected.insert(expected.end(), 3);
    expected.insert(expected.end(), 4);
    expected.insert(expected.end(), 1);
    expected.insert(expected.end(), 2);

    tree.move_nodes(tree.end(),
                    SourcePosition{2},
                    Count{2},
                    tree.end(),
                    DestinationPosition{0});

    EXPECT_THAT(tree, testing::ElementsAre(3, 4, 1, 2));
    EXPECT_EQ(expected, tree);
}

TEST_F(TestTreeFixture, moving_nodes_in_reparenting_combinations)
{
    /*
     * 1
     * 2
     * 3
     * 4
     */

    /*
     * 2
     *   1
     * 3
     * 4
     */

    /*
     * 2
     *   1
     * 4
     *   3
     */
    ds::Tree<int> tree;
    tree.insert(tree.end(), 1);
    tree.insert(tree.end(), 2);
    tree.insert(tree.end(), 3);
    tree.insert(tree.end(), 4);
    ds::Tree<int> expected;
    expected.insert(expected.insert(expected.end(), 2), 1);
    expected.insert(expected.insert(expected.end(), 4), 3);

    using std::ranges::find;
    tree.move_nodes(tree.end(),
                    SourcePosition{0},
                    Count{1},
                    find(tree, 2),
                    DestinationPosition{0});
    tree.move_nodes(tree.end(),
                    SourcePosition{1},
                    Count{1},
                    find(tree, 4),
                    DestinationPosition{0});

    EXPECT_EQ(expected, tree);
}

TEST_F(TestTreeFixture, moving_nodes_between_roots_from_left_to_mid)
{
    /*
     *
     * 1
     *   2
     *   3
     *     10
     *        11
     *   4
     *   5
     * 6
     *   7
     *   8
     *   9
     *   12
     */

    /*
     *
     * 1
     *   4
     *   5
     * 6
     *   7
     *   8
     *   2
     *   3
     *     10
     *        11
     *   9
     *   12
     */

    auto tree = make_move_testing_tree();
    auto source_parent = std::ranges::find(tree, 1);
    auto destination_parent = std::ranges::find(tree, 6);

    std::cout << tree.to_string() << std::endl;
    std::cout << std::endl;

    tree.move_nodes(source_parent,
                    SourcePosition{0},
                    Count{2},
                    destination_parent,
                    DestinationPosition{2});

    std::cout << tree.to_string() << std::endl;
    EXPECT_THAT(tree,
                ::testing::ElementsAre(1, 4, 5, 6, 7, 8, 2, 3, 10, 11, 9, 12));
}

TEST_F(TestTreeFixture, moving_nodes_between_roots_from_right_to_left)
{
    /*
     *
     * 1
     *   2
     *   3
     *     10
     *        11
     *   4
     *   5
     * 6
     *   7
     *   8
     *   9
     *   12
     */

    /*
     *
     * 1
     *   2
     *   3
     *     10
     *        11
     * 6
     *   4
     *   5
     *   7
     *   8
     *   9
     *   12
     */
    auto tree = make_move_testing_tree();
    auto source_parent = std::ranges::find(tree, 1);
    auto destination_parent = std::ranges::find(tree, 6);

    tree.move_nodes(source_parent,
                    SourcePosition{2},
                    Count{2},
                    destination_parent,
                    DestinationPosition{0});

    EXPECT_THAT(tree,
                ::testing::ElementsAre(1, 2, 3, 10, 11, 6, 4, 5, 7, 8, 9, 12));
}

TEST_F(TestTreeFixture, moving_nodes_between_roots_from_mid_to_right)
{
    /*
     *
     * 1
     *   2
     *   3
     *     10
     *        11
     *   4
     *   5
     * 6
     *   7
     *   8
     *   9
     *   12
     */

    /*
     *
     * 1
     *   2
     *   5
     * 6
     *   7
     *   8
     *   9
     *   12
     *   3
     *     10
     *        11
     *   4
     */
    auto tree = make_move_testing_tree();
    auto source_parent = std::ranges::find(tree, 1);
    auto destination_parent = std::ranges::find(tree, 6);

    tree.move_nodes(source_parent,
                    SourcePosition{1},
                    Count{2},
                    destination_parent,
                    DestinationPosition{4});

    EXPECT_THAT(tree,
                ::testing::ElementsAre(1, 2, 5, 6, 7, 8, 9, 12, 3, 10, 11, 4));
}

TEST_F(TestTreeFixture, following_parents)
{
    const auto tree = make_sample_tree();
    auto it = std::ranges::find(tree, 9);

    it = tree.parent(it);
    EXPECT_EQ(8, *it);
    it = tree.parent(it);
    EXPECT_EQ(7, *it);
    it = tree.parent(it);
    EXPECT_EQ(12, *it);
    it = tree.parent(it);
    EXPECT_EQ(1, *it);
    it = tree.parent(it);
    EXPECT_EQ(tree.end(), it);
}

TEST_F(TestTreeFixture, browsing_children)
{
    const auto tree = make_sample_tree();
    const std::vector<int> expected{4, 5};

    const auto children = tree.children(std::ranges::find(tree, 3));

    EXPECT_TRUE(std::ranges::equal(expected, children));
}

TEST_F(TestTreeFixture, returns_root_children_when_given_end_iterator)
{
    const auto tree = make_sample_tree();
    const std::vector<int> expected{1};

    const auto children = tree.children(tree.cend());

    EXPECT_TRUE(std::ranges::equal(expected, children));
}

TEST_F(TestTreeFixture, returns_node_position_in_children) {
    EXPECT_EQ(1, sut.position_in_children(std::ranges::find(sut, 5)));
    EXPECT_EQ(0, sut.position_in_children(std::ranges::find(sut, 1)));
    EXPECT_EQ(0, sut.position_in_children(sut.cend()));
}
