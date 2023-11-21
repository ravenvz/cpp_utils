#include "cpp_utils/datastructures/TreeMap.h"
#include "gmock/gmock.h"

ds::TreeMap<std::string, int> make_sample_tree()
{
    using ds::TreeMap;
    TreeMap<std::string, int> sut;
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
    sut.addChild("1", 1, std::nullopt);
    sut.addChild("2", 2, "1");
    sut.addChild("3", 3, "1");
    sut.addChild("4", 4, std::nullopt);
    sut.addChild("5", 5, "4");
    sut.addChild("6", 6, "5");
    sut.addChild("7", 7, "5");
    sut.addChild("8", 8, "7");
    sut.addChild("9", 9, std::nullopt);
    sut.addChild("10", 10, "2");
    return sut;
}

class TreeMapFixture : public ::testing::Test {
public:
    ds::TreeMap<std::string, int> sut{make_sample_tree()};
};

TEST_F(TreeMapFixture, is_copy_constructible)
{
    const auto actual = sut;

    EXPECT_EQ(sut, actual);
}

TEST_F(TreeMapFixture, is_copy_assignable)
{
    ds::TreeMap<std::string, int> actual;

    actual = sut;

    EXPECT_EQ(sut, actual);
}

TEST_F(TreeMapFixture, throws_when_adding_child_with_non_unique_key)
{
    /*
     * 1
     *   2
     *   3
     */

    ds::TreeMap<std::string, int> tree;
    tree.addChild("1", 1, std::nullopt);
    tree.addChild("2", 2, "1");
    tree.addChild("3", 3, "1");

    ASSERT_THROW(tree.addChild("3", 77, std::nullopt), std::runtime_error);
}

TEST_F(TreeMapFixture, able_to_transform_tree_to_tree_with_another_payload_type)
{
    ds::TreeMap<std::string, std::string> expected;
    expected.addChild("1", "1", std::nullopt);
    expected.addChild("2", "4", "1");
    expected.addChild("3", "9", "1");
    expected.addChild("4", "16", std::nullopt);
    expected.addChild("5", "25", "4");
    expected.addChild("6", "36", "5");
    expected.addChild("7", "49", "5");
    expected.addChild("8", "64", "7");
    expected.addChild("9", "81", std::nullopt);
    expected.addChild("10", "100", "2");
    auto square = [](const int& x) { return std::to_string(x * x); };

    ds::TreeMap<std::string, std::string> actual{sut.mapped(square)};

    EXPECT_EQ(expected, actual);
}

TEST_F(TreeMapFixture, comparing_trees_with_custom_compare_function)
{
    const auto lhs = [] {
        ds::TreeMap<std::string, int> tree;
        tree.addChild("1", 7, std::nullopt);
        tree.addChild("2", 7, "1");
        tree.addChild("3", 7, std::nullopt);
        tree.addChild("4", 7, "3");
        return tree;
    }();
    const auto rhs = [] {
        ds::TreeMap<std::string, int> tree;
        tree.addChild("11", 7, std::nullopt);
        tree.addChild("22", 7, "11");
        tree.addChild("33", 7, std::nullopt);
        tree.addChild("44", 7, "33");
        return tree;
    }();

    EXPECT_TRUE(compare(lhs, rhs, [](const auto& left, const auto& right) {
        return left.second == right.second;
    }));
    EXPECT_FALSE(compare(lhs, rhs));
}

TEST_F(TreeMapFixture, add_child_throws_when_parent_is_bogus)
{
    ds::TreeMap<std::string, int> tree;
    tree.addChild("1", 1, std::nullopt);

    EXPECT_THROW(tree.addChild("2", 2, "3"), std::runtime_error);
}

struct UncomparableData {
    int val;
};

template <class CharT, class Traits>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const UncomparableData& data)
{
    os << data.val;
    return os;
}

bool operator==(const UncomparableData& lhs, const UncomparableData& rhs)
{
    return lhs.val == rhs.val;
}

TEST_F(TreeMapFixture, flatten_and_unflatten_empty_tree)
{
    ds::TreeMap<int, UncomparableData> tree;
    const auto flattened = tree.flatten();

    const auto restored =
        ds::TreeMap<int, UncomparableData>::unflatten(flattened);

    EXPECT_EQ(tree, restored);
}

TEST_F(TreeMapFixture, flatten_and_unflatten)
{
    ds::TreeMap<int, UncomparableData> tree;
    tree.addChild(1, UncomparableData{1}, std::nullopt);
    tree.addChild(2, UncomparableData{2}, 1);
    tree.addChild(3, UncomparableData{3}, 1);
    tree.addChild(4, UncomparableData{4}, std::nullopt);
    tree.addChild(5, UncomparableData{5}, 4);
    tree.addChild(6, UncomparableData{6}, 5);
    tree.addChild(7, UncomparableData{7}, 5);
    tree.addChild(8, UncomparableData{8}, 7);
    tree.addChild(9, UncomparableData{9}, std::nullopt);
    tree.addChild(10, UncomparableData{10}, 2);

    const auto flattened = tree.flatten();
    const auto restored =
        ds::TreeMap<int, UncomparableData>::unflatten(flattened);

    EXPECT_EQ(tree, restored);
}

TEST_F(TreeMapFixture, returns_none_when_asked_for_payload_for_missing_key)
{
    EXPECT_EQ(std::nullopt, sut.payload("bogus_key"));
}

TEST_F(TreeMapFixture, returns_payload_for_given_key)
{
    EXPECT_EQ(8, sut.payload("8"));
    EXPECT_EQ(1, sut.payload("1"));
}

TEST_F(TreeMapFixture, thows_when_asked_for_parent_of_bogus_child)
{
    EXPECT_THROW(sut.parent("bogus_id"), std::runtime_error);
}

TEST_F(TreeMapFixture, returns_null_when_asked_for_parent_of_top_level_child)
{
    EXPECT_EQ(std::nullopt, sut.parent("4"));
}

TEST_F(TreeMapFixture,
       returns_some_key_when_asked_for_parent_of_lower_level_child)
{
    EXPECT_EQ("7", sut.parent("8").value().get());
}

TEST_F(TreeMapFixture, returns_top_level_when_asked_for_children_for_bogus_key)
{
    const std::vector<std::string> expected{"1", "4", "9"};

    EXPECT_TRUE(std::ranges::equal(expected, sut.children("bogus_key")));
}

TEST_F(TreeMapFixture, returns_top_level_children)
{
    const std::vector<std::string> expected{"1", "4", "9"};

    EXPECT_TRUE(std::ranges::equal(expected, sut.children()));
}

TEST_F(TreeMapFixture, returns_children_for_existing_key)
{
    const std::vector<std::string> expected{"6", "7"};

    EXPECT_TRUE(std::ranges::equal(expected, sut.children("5")));
}

TEST_F(TreeMapFixture, returns_none_when_asked_for_nth_child_of_missing_key)
{
    EXPECT_EQ(std::nullopt, sut.nthChild("bogus_key", 0));
}

TEST_F(TreeMapFixture,
       returns_none_when_asked_for_nth_child_of_existing_key_but_out_of_bounds)
{

    EXPECT_EQ(std::nullopt, sut.nthChild("5", 2));
    EXPECT_EQ(std::nullopt, sut.nthChild(3));
}

TEST_F(TreeMapFixture, returns_nth_child)
{
    EXPECT_EQ(7, sut.nthChild("5", 1));
    EXPECT_EQ(9, sut.nthChild(2));
}

TEST_F(TreeMapFixture, returns_node_position_in_parent_children)
{
    EXPECT_EQ(std::optional<size_t>{2}, sut.positionInChildren("9"));
    EXPECT_EQ(std::optional<size_t>{1}, sut.positionInChildren("7"));
    EXPECT_EQ(std::optional<size_t>{0}, sut.positionInChildren("5"));
    EXPECT_EQ(std::optional<size_t>{}, sut.positionInChildren("bogus_key"));
}

TEST_F(TreeMapFixture, returns_keys_in_unspecified_order)
{
    std::vector<std::string> expected{
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
    std::vector<std::string> actual;
    std::ranges::copy(sut.keys(), std::back_inserter(actual));

    EXPECT_THAT(actual, ::testing::UnorderedElementsAreArray(expected));
}

TEST_F(TreeMapFixture, removing_root_nodes)
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
     *     11
     *     12
     *       14
     *     13
     * 9
     */

    /*
     * 1
     *   2
     *     10
     *   3
     * 4
     *   5
     *     6
     *     12
     *       14
     *     13
     * 9
     */
    sut.addChild("11", 11, "5");
    sut.addChild("12", 12, "5");
    sut.addChild("14", 14, "12");
    sut.addChild("13", 13, "5");
    ds::TreeMap<std::string, int> expected;
    expected.addChild("1", 1, std::nullopt);
    expected.addChild("2", 2, "1");
    expected.addChild("10", 10, "2");
    expected.addChild("3", 3, "1");
    expected.addChild("4", 4, std::nullopt);
    expected.addChild("5", 5, "4");
    expected.addChild("6", 6, "5");
    expected.addChild("12", 12, "5");
    expected.addChild("14", 14, "12");
    expected.addChild("13", 13, "5");
    expected.addChild("9", 9, std::nullopt);

    sut.removeNodes("5", 1, 2);

    EXPECT_EQ(expected, sut);
    std::vector<std::string> keys;
    std::ranges::copy(sut.keysView(), std::back_inserter(keys));
    EXPECT_THAT(keys,
                ::testing::UnorderedElementsAre(
                    "1", "2", "3", "4", "5", "6", "9", "10", "12", "13", "14"));
}

TEST_F(TreeMapFixture, removing_nodes_also_removes_all_subnodes)
{
    /*
     * 1
     *   2
     *     3
     *     4
     *       5
     *         6
     *           7
     *     8
     *     9
     * 10
     */

    /*
     * 1
     *   2
     *     3
     *     8
     *     9
     * 10
     */
    ds::TreeMap<std::string, int> tree;
    tree.addChild("1", 1, std::nullopt);
    tree.addChild("2", 2, "1");
    tree.addChild("3", 3, "2");
    tree.addChild("4", 4, "2");
    tree.addChild("5", 5, "4");
    tree.addChild("6", 6, "5");
    tree.addChild("7", 7, "6");
    tree.addChild("8", 8, "2");
    tree.addChild("9", 9, "2");
    tree.addChild("10", 10, std::nullopt);
    ds::TreeMap<std::string, int> expected;
    expected.addChild("1", 1, std::nullopt);
    expected.addChild("2", 2, "1");
    expected.addChild("3", 3, "2");
    expected.addChild("8", 8, "2");
    expected.addChild("9", 9, "2");
    expected.addChild("10", 10, std::nullopt);

    tree.removeNodes("2", 1, 1);

    EXPECT_EQ(expected, tree);
    std::vector<std::string> keys;
    std::ranges::copy(tree.keysView(), std::back_inserter(keys));
    EXPECT_THAT(keys,
                ::testing::UnorderedElementsAre("1", "2", "3", "8", "9", "10"));
}

TEST_F(TreeMapFixture, removing_nodes_throws_when_count_is_too_large)
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
    EXPECT_THROW(sut.removeNodes("5", 1, 3), std::runtime_error);
}

TEST_F(TreeMapFixture, removing_nodes_throws_when_row_specified_does_not_exists)
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
    EXPECT_THROW(sut.removeNodes("5", 3, 1), std::runtime_error);
}

TEST_F(TreeMapFixture, remove_node_throws_when_key_does_not_exists)
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
    EXPECT_THROW(sut.removeNode("77"), std::runtime_error);
}

TEST_F(TreeMapFixture, remove_node)
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

    /*
     * 1
     *   2
     *     10
     *   3
     * 4
     *   5
     *     6
     * 9
     */
    ds::TreeMap<std::string, int> tree;
    tree.addChild("1", 1, std::nullopt);
    tree.addChild("2", 2, "1");
    tree.addChild("10", 10, "2");
    tree.addChild("3", 3, "1");
    tree.addChild("4", 4, std::nullopt);
    tree.addChild("5", 5, "4");
    tree.addChild("6", 6, "5");
    tree.addChild("9", 9, std::nullopt);

    sut.removeNode("7");

    EXPECT_EQ(tree, sut);
}

TEST_F(TreeMapFixture, moving_nodes_between_leaves)
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

    /*
     * 1
     * 4
     *   5
     *     6
     *     2
     *       10
     *     3
     *     7
     *       8
     * 9
     */
    ds::TreeMap<std::string, int> expected;
    expected.addChild("1", 1, std::nullopt);
    expected.addChild("4", 4, std::nullopt);
    expected.addChild("5", 5, "4");
    expected.addChild("6", 6, "5");
    expected.addChild("2", 2, "5");
    expected.addChild("3", 3, "5");
    expected.addChild("10", 10, "2");
    expected.addChild("7", 7, "5");
    expected.addChild("8", 8, "7");
    expected.addChild("9", 9, std::nullopt);

    sut.moveNodes("1", 0, 2, "5", 1);

    EXPECT_EQ(expected, sut);
}

TEST_F(TreeMapFixture, moving_root_nodes_within_root_itself)
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
    ds::TreeMap<std::string, int> actual;
    actual.addChild("1", 1, std::nullopt);
    actual.addChild("2", 2, std::nullopt);
    actual.addChild("3", 3, std::nullopt);
    actual.addChild("4", 4, std::nullopt);

    ds::TreeMap<std::string, int> expected;
    expected.addChild("4", 4, std::nullopt);
    expected.addChild("1", 1, std::nullopt);
    expected.addChild("2", 2, std::nullopt);
    expected.addChild("3", 3, std::nullopt);

    actual.moveNodes(std::nullopt, 0, 3, std::nullopt, 4);

    EXPECT_EQ(expected, actual);
}

TEST_F(TreeMapFixture, moving_root_node_down_within_root_itself)
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
    ds::TreeMap<std::string, int> actual;
    actual.addChild("1", 1, std::nullopt);
    actual.addChild("2", 2, std::nullopt);
    actual.addChild("3", 3, std::nullopt);
    actual.addChild("4", 4, std::nullopt);

    ds::TreeMap<std::string, int> expected;
    expected.addChild("2", 2, std::nullopt);
    expected.addChild("3", 3, std::nullopt);
    expected.addChild("1", 1, std::nullopt);
    expected.addChild("4", 4, std::nullopt);

    actual.moveNodes(std::nullopt, 0, 1, std::nullopt, 3);

    EXPECT_EQ(expected, actual);
}

TEST_F(TreeMapFixture, moving_root_nodes_in_reparenting_combinations)
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

    ds::TreeMap<std::string, int> actual;
    actual.addChild("1", 1, std::nullopt);
    actual.addChild("2", 2, std::nullopt);
    actual.addChild("3", 3, std::nullopt);
    actual.addChild("4", 4, std::nullopt);

    ds::TreeMap<std::string, int> expected;
    expected.addChild("2", 2, std::nullopt);
    expected.addChild("1", 1, "2");
    expected.addChild("4", 4, std::nullopt);
    expected.addChild("3", 3, "4");

    actual.moveNodes(std::nullopt, 0, 1, "2", 0);
    actual.moveNodes(std::nullopt, 1, 1, "4", 0);

    EXPECT_EQ("2", actual.parent("1").value().get());
    EXPECT_EQ(std::nullopt, actual.parent("2"));
    EXPECT_EQ("4", actual.parent("3").value().get());
    EXPECT_EQ(std::nullopt, actual.parent("4"));
    EXPECT_EQ(expected, actual);
}

TEST_F(TreeMapFixture, test_extensive_reparenting)
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
     * 4
     *   3
     */

    /*
     * 2
     * 4
     *   1
     *   3
     */

    ds::TreeMap<std::string, int> actual;
    actual.addChild("1", 1, std::nullopt);
    actual.addChild("2", 2, std::nullopt);
    actual.addChild("3", 3, std::nullopt);
    actual.addChild("4", 4, std::nullopt);

    ds::TreeMap<std::string, int> expected;
    expected.addChild("2", 2, std::nullopt);
    expected.addChild("4", 4, std::nullopt);
    expected.addChild("1", 1, "4");
    expected.addChild("3", 3, "4");

    actual.moveNodes(std::nullopt, 0, 1, "2", 0);
    actual.moveNodes(std::nullopt, 1, 1, "4", 0);
    actual.moveNodes("2", 0, 1, "4", 0);

    EXPECT_EQ("4", actual.parent("1").value().get());
    EXPECT_EQ(std::nullopt, actual.parent("2"));
    EXPECT_EQ("4", actual.parent("3").value().get());
    EXPECT_EQ(std::nullopt, actual.parent("4"));
    EXPECT_EQ(expected, actual);
}

TEST_F(TreeMapFixture, moving_nodes_within_same_parent)
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

    /*
     * 1
     *   2
     *     10
     *   3
     * 4
     *   5
     *     7
     *       8
     *     6
     * 9
     */
    ds::TreeMap<std::string, int> expected;
    expected.addChild("1", 1, std::nullopt);
    expected.addChild("2", 2, "1");
    expected.addChild("10", 10, "2");
    expected.addChild("3", 3, "1");
    expected.addChild("4", 4, std::nullopt);
    expected.addChild("5", 5, "4");
    expected.addChild("7", 7, "5");
    expected.addChild("8", 8, "7");
    expected.addChild("6", 6, "5");
    expected.addChild("9", 9, std::nullopt);

    sut.moveNodes("5", 0, 1, "5", 2);

    EXPECT_EQ(expected, sut);
}

TEST_F(TreeMapFixture, moving_nodes_between_non_existing_node_throws)
{
    EXPECT_THROW(sut.moveNodes("100500", 0, 1, "2", 0), std::runtime_error);
    EXPECT_THROW(sut.moveNodes("2", 0, 1, "1000400", 0), std::runtime_error);
}

TEST_F(TreeMapFixture, moving_nodes_with_bogus_source_row_throws)
{
    EXPECT_THROW(sut.moveNodes("1", 5, 1, "2", 0), std::runtime_error);
}

TEST_F(TreeMapFixture, moving_nodes_to_non_existing_position_appends)
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

    /*
     * 1
     *   2
     *     10
     * 4
     *   5
     *     6
     *     7
     *       8
     *     3
     * 9
     */
    ds::TreeMap<std::string, int> expected;
    expected.addChild("1", 1, std::nullopt);
    expected.addChild("2", 2, "1");
    expected.addChild("10", 10, "2");
    expected.addChild("4", 4, std::nullopt);
    expected.addChild("5", 5, "4");
    expected.addChild("6", 6, "5");
    expected.addChild("7", 7, "5");
    expected.addChild("8", 8, "7");
    expected.addChild("3", 3, "5");
    expected.addChild("9", 9, std::nullopt);

    sut.moveNodes("1", 1, 1, "5", 200);

    EXPECT_EQ(expected, sut);
}

TEST_F(TreeMapFixture, sub_tree)
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
    ds::TreeMap<std::string, int> expected;
    expected.addChild("5", 5, std::nullopt);
    expected.addChild("6", 6, "5");
    expected.addChild("7", 7, "5");
    expected.addChild("8", 8, "7");

    const auto actual = sut.subTreeMap("5");

    EXPECT_EQ(expected, actual);
}

TEST_F(TreeMapFixture, subtree_from_leaf)
{

    /*   0
     *      1
     *        2
     *        5
     *        3
     *    4
     */
    ds::TreeMap<std::string, int> tree;
    tree.addChild("0", 0, std::nullopt);
    tree.addChild("1", 1, "0");
    tree.addChild("2", 2, "1");
    tree.addChild("5", 5, "1");
    tree.addChild("3", 3, "1");
    tree.addChild("4", 4, std::nullopt);
    ds::TreeMap<std::string, int> expected;
    expected.addChild("3", 3, std::nullopt);

    const auto actual = tree.subTreeMap("3");

    EXPECT_EQ(expected, actual);
}

TEST_F(TreeMapFixture, add_subtree)
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

    /*
     * 1
     *   2
     *     10
     *       11
     *       12
     *         13
     *   3
     * 4
     *   5
     *     6
     *     7
     *       8
     * 9
     */
    ds::TreeMap<std::string, int> expected;
    expected.addChild("1", 1, std::nullopt);
    expected.addChild("2", 2, "1");
    expected.addChild("10", 10, "2");
    expected.addChild("3", 3, "1");
    expected.addChild("4", 4, std::nullopt);
    expected.addChild("5", 5, "4");
    expected.addChild("6", 6, "5");
    expected.addChild("7", 7, "5");
    expected.addChild("8", 8, "7");
    expected.addChild("9", 9, std::nullopt);
    expected.addChild("11", 11, "10");
    expected.addChild("12", 12, "10");
    expected.addChild("13", 13, "12");
    ds::TreeMap<std::string, int> addedTreeMap;
    addedTreeMap.addChild("11", 11, std::nullopt);
    addedTreeMap.addChild("12", 12, std::nullopt);
    addedTreeMap.addChild("13", 13, "12");

    sut.addSubtree(addedTreeMap, "10", 0);

    EXPECT_EQ(expected, sut);
}

TEST_F(TreeMapFixture, add_subtree_to_top_level)
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
     * 11
     *   12
     *     13
     */
    ds::TreeMap<std::string, int> expected;
    expected.addChild("1", 1, std::nullopt);
    expected.addChild("2", 2, "1");
    expected.addChild("10", 10, "2");
    expected.addChild("3", 3, "1");
    expected.addChild("4", 4, std::nullopt);
    expected.addChild("5", 5, "4");
    expected.addChild("6", 6, "5");
    expected.addChild("7", 7, "5");
    expected.addChild("8", 8, "7");
    expected.addChild("9", 9, std::nullopt);
    expected.addChild("11", 11, std::nullopt);
    expected.addChild("12", 12, "11");
    expected.addChild("13", 13, "12");
    ds::TreeMap<std::string, int> tree;
    tree.addChild("11", 11, std::nullopt);
    tree.addChild("12", 12, "11");
    tree.addChild("13", 13, "12");

    sut.addSubtree(tree, std::nullopt, 3);

    EXPECT_EQ(expected, sut);
}

TEST_F(TreeMapFixture, entries_view)
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

    ds::TreeMap<std::string, int> tree;
    tree.addChild("1", 1, std::nullopt);
    tree.addChild("2", 2, "1");
    tree.addChild("10", 10, "2");
    tree.addChild("3", 3, "1");
    tree.addChild("4", 4, std::nullopt);
    tree.addChild("5", 5, "4");
    tree.addChild("7", 7, "5");
    tree.addChild("8", 8, "7");
    tree.addChild("6", 6, "5");
    tree.addChild("9", 9, std::nullopt);

    std::vector<std::string> keys;
    std::ranges::copy(tree.keysView(), std::back_inserter(keys));
    std::vector<int> payload;
    std::ranges::copy(tree.payloadView(), std::back_inserter(payload));
    std::vector<std::pair<std::string, int>> entries;
    std::ranges::copy(tree.entriesView(), std::back_inserter(entries));

    EXPECT_THAT(keys,
                ::testing::UnorderedElementsAre(
                    "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"));
    EXPECT_THAT(payload,
                ::testing::UnorderedElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
    EXPECT_THAT(entries,
                ::testing::UnorderedElementsAre(::testing::Pair("1", 1),
                                                ::testing::Pair("2", 2),
                                                ::testing::Pair("3", 3),
                                                ::testing::Pair("4", 4),
                                                ::testing::Pair("5", 5),
                                                ::testing::Pair("6", 6),
                                                ::testing::Pair("7", 7),
                                                ::testing::Pair("8", 8),
                                                ::testing::Pair("9", 9),
                                                ::testing::Pair("10", 10)));
}
