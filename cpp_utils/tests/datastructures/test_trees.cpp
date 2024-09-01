#include "cpp_utils/datastructures/LinearTree.h"
#include "cpp_utils/datastructures/Tree.h"
#include "gmock/gmock.h"
#include <ranges>
#include <tuple>

using ::testing::ElementsAre;

using namespace ds;

struct CompoundType {
    int some_value;
    std::string id;
};

template <typename ParamTuple>
class GenericTreeFixture : public ::testing::Test {
public:
    using IntTree = typename std::tuple_element_t<0, ParamTuple>;
    using StringTree = typename std::tuple_element_t<1, ParamTuple>;
    using CompoundTree = typename std::tuple_element_t<2, ParamTuple>;

    IntTree sut = this->make_sample_tree();
    IntTree inbox_tree = this->make_inbox_like_tree();

    auto make_sample_tree() -> IntTree
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
        auto tree = IntTree::from_flattened(flattened);
        return tree;
    }

    auto make_move_testing_tree() -> IntTree
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
        IntTree tree;
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

    auto make_multiroot_sample_tree() -> IntTree
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
        IntTree tree;

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

    auto make_inbox_like_tree() -> IntTree
    {
        /*
         * 1
         *   2
         *   3
         *   4
         *   5
         *   6
         *   7
         *   8
         */
        IntTree tree;
        auto one = tree.insert(tree.end(), 1);
        tree.insert(one, 2);
        tree.insert(one, 3);
        tree.insert(one, 4);
        tree.insert(one, 5);
        tree.insert(one, 6);
        tree.insert(one, 7);
        tree.insert(one, 8);

        return tree;
    }
};

using MyTypes = ::testing::Types<
    std::tuple<Tree<int>, Tree<std::string>, Tree<CompoundType>>,
    std::tuple<LinearTree<int>,
               LinearTree<std::string>,
               LinearTree<CompoundType>>>;
TYPED_TEST_SUITE(GenericTreeFixture, MyTypes);

TYPED_TEST(GenericTreeFixture, flatten_and_unflatten_inbox_tree)
{
    std::vector<std::optional<int>> expected_flattened{std::nullopt,
                                                       std::nullopt,
                                                       1,
                                                       std::nullopt,
                                                       2,
                                                       3,
                                                       4,
                                                       5,
                                                       6,
                                                       7,
                                                       8,
                                                       std::nullopt,
                                                       std::nullopt,
                                                       std::nullopt,
                                                       std::nullopt,
                                                       std::nullopt,
                                                       std::nullopt,
                                                       std::nullopt,
                                                       std::nullopt};

    EXPECT_EQ(expected_flattened, this->inbox_tree.flatten());
}

TYPED_TEST(GenericTreeFixture, dfs_iteration)
{
    EXPECT_THAT(this->sut, ElementsAre(1, 2, 10, 3, 4, 5, 6, 7, 8, 9));
}

TYPED_TEST(GenericTreeFixture, begin_end_iterators_of_empty_tree)
{
    typename TestFixture::StringTree tree;

    EXPECT_EQ(tree.begin(), tree.end());
}

TYPED_TEST(GenericTreeFixture, cbegin_cend_iterators_of_empty_tree)
{
    typename TestFixture::StringTree tree;

    EXPECT_EQ(tree.cbegin(), tree.cend());
}

TYPED_TEST(GenericTreeFixture, accessing_iterator)
{
    typename TestFixture::CompoundTree tree;
    tree.insert(tree.end(), CompoundType{2, "2"});

    auto it = tree.cbegin();

    EXPECT_EQ((*it).id, it->id);
    EXPECT_EQ("2", (*it).id);
    EXPECT_EQ("2", it->id);
}

TYPED_TEST(GenericTreeFixture, returns_tree_size)
{
    ASSERT_EQ(10, this->sut.size());
}

TYPED_TEST(GenericTreeFixture, returns_if_tree_empty)
{
    typename TestFixture::IntTree tree;

    EXPECT_TRUE(tree.empty());
    EXPECT_FALSE(this->sut.empty());
}

TYPED_TEST(GenericTreeFixture, transforming_tree)
{
    auto mapped_tree = this->sut.transform(
        [](auto val) -> std::string { return std::to_string(val); });

    EXPECT_THAT(mapped_tree,
                ElementsAre("1", "2", "10", "3", "4", "5", "6", "7", "8", "9"));
}

TYPED_TEST(GenericTreeFixture, transforming_tree_with_projection)
{
    /*
     * 1 "abc"
     *   7 "cde"
     *     2 "efg"
     * 5 "ghi"
     *   8 "ijk"
     */

    /* Transformed
     *
     * 1
     *   49
     *     4
     * 25
     *   64
     */
    auto square = [](int x) { return x * x; };
    auto value_projection = [](const CompoundType& compound) {
        return compound.some_value;
    };
    typename TestFixture::CompoundTree tree;
    auto it = tree.insert(tree.end(), CompoundType{1, "abc"});
    auto it2 = tree.insert(it, CompoundType{7, "cde"});
    tree.insert(it2, CompoundType{2, "efg"});
    auto it3 = tree.insert(tree.end(), CompoundType{5, "ghi"});
    tree.insert(it3, CompoundType{8, "ijk"});
    typename TestFixture::IntTree expected;
    auto e_it = expected.insert(expected.end(), 1);
    auto e_2 = expected.insert(e_it, 49);
    expected.insert(e_2, 4);
    auto e_3 = expected.insert(expected.end(), 25);
    expected.insert(e_3, 64);

    auto mapped_tree = tree.transform(tree.cend(), square, value_projection);

    EXPECT_EQ(expected, mapped_tree);
}

TYPED_TEST(GenericTreeFixture,
           iterator_does_not_in_fact_iterates_over_subtree_only)
{
    auto it = this->sut.insert(std::ranges::find(this->sut, 10), 77);
    std::vector<int> actual;
    std::copy(it, this->sut.end(), std::back_inserter(actual));

    EXPECT_THAT(actual, ElementsAre(77, 3, 4, 5, 6, 7, 8, 9));
}

TYPED_TEST(GenericTreeFixture, map_subtree)
{
    this->sut.map(std::ranges::find(this->sut, 5),
                  [](auto& val) { val *= val; });

    EXPECT_THAT(this->sut, ElementsAre(1, 2, 10, 3, 4, 25, 36, 49, 64, 9));
}

TYPED_TEST(GenericTreeFixture, map_root)
{
    this->sut.map(this->sut.end(), [](auto& val) { val *= val; });

    EXPECT_THAT(this->sut, ElementsAre(1, 4, 100, 9, 16, 25, 36, 49, 64, 81));
}

TYPED_TEST(GenericTreeFixture, returns_node_position_in_children)
{
    EXPECT_EQ(1,
              this->sut.position_in_children(std::ranges::find(this->sut, 7)));
    EXPECT_EQ(0,
              this->sut.position_in_children(std::ranges::find(this->sut, 1)));
    EXPECT_EQ(0, this->sut.position_in_children(this->sut.cend()));
}

TYPED_TEST(GenericTreeFixture, following_parents)
{
    auto it = std::ranges::find(this->sut, 8);
    it = this->sut.parent(it);
    EXPECT_EQ(7, *it);
    it = this->sut.parent(it);
    EXPECT_EQ(5, *it);
    it = this->sut.parent(it);
    EXPECT_EQ(4, *it);
    it = this->sut.parent(it);
    EXPECT_EQ(this->sut.cend(), it);
}

TYPED_TEST(GenericTreeFixture, provides_view_to_children)
{
    const std::vector<int> expected{6, 7};

    const auto children = this->sut.children(std::ranges::find(this->sut, 5));

    EXPECT_TRUE(std::ranges::equal(expected, children));
}

TYPED_TEST(GenericTreeFixture, provides_view_to_root_children)
{
    const std::vector<int> expected{1, 4, 9};

    const auto children = this->sut.children(this->sut.cend());

    EXPECT_TRUE(std::ranges::equal(expected, children));
}

TYPED_TEST(GenericTreeFixture, erasing_nodes)
{
    auto tree = TestFixture::make_move_testing_tree();

    tree.erase(std::ranges::find(tree, 6));

    EXPECT_THAT(tree, ElementsAre(1, 2, 3, 10, 11, 4, 5));
}

TYPED_TEST(GenericTreeFixture,
           erasing_nodes_does_nothing_if_end_iterator_is_passed)
{
    this->sut.erase(this->sut.end());

    EXPECT_THAT(this->sut, ElementsAre(1, 2, 10, 3, 4, 5, 6, 7, 8, 9));
}

TYPED_TEST(GenericTreeFixture, erasing_and_inserting_nodes)
{
    /*
     * 1
     *   77
     *   3
     * 4
     * 9
     *   22
     * 10
     * 11
     * 12
     * 13
     * 14
     * 15
     */
    this->sut.erase(std::ranges::find(this->sut, 2));
    this->sut.erase(std::ranges::find(this->sut, 5));
    this->sut.insert(
        std::ranges::find(this->sut, 1), 77, DestinationPosition{0});
    this->sut.insert(std::ranges::find(this->sut, 9), 22);
    for (int i = 0; i < 5; ++i) {
        this->sut.insert(this->sut.end(), i + 10);
    }

    EXPECT_THAT(this->sut, ElementsAre(1, 77, 3, 4, 9, 22, 10, 11, 12, 13, 14));
}

TYPED_TEST(GenericTreeFixture, inserting_at_position)
{
    auto actual = this->sut.insert(
        std::ranges::find(this->sut, 5), 77, DestinationPosition{1});

    EXPECT_EQ(std::ranges::find(this->sut, 77), actual);
    EXPECT_THAT(this->sut, ElementsAre(1, 2, 10, 3, 4, 5, 6, 77, 7, 8, 9));
}

TYPED_TEST(GenericTreeFixture, inserting_at_optional_position)
{
    std::optional<DestinationPosition> maybe_pos{1};
    auto actual =
        this->sut.insert(std::ranges::find(this->sut, 5), 77, maybe_pos);

    EXPECT_EQ(std::ranges::find(this->sut, 77), actual);
    EXPECT_THAT(this->sut, ElementsAre(1, 2, 10, 3, 4, 5, 6, 77, 7, 8, 9));
}

TYPED_TEST(GenericTreeFixture, inserting_throws_when_destination_out_of_range)
{
    EXPECT_THROW(this->sut.insert(std::ranges::find(this->sut, 5),
                                  77,
                                  DestinationPosition{3}),
                 std::out_of_range);
    EXPECT_THROW(this->sut.insert(std::ranges::find(this->sut, 5),
                                  77,
                                  DestinationPosition{-1}),
                 std::out_of_range);
}

TYPED_TEST(GenericTreeFixture, batch_insert)
{
    /*
     * 1
     *   2
     *     10
     *   3
     * 4
     *   5
     *     6
     *       20
     *       21
     *       22
     *       23
     *       24
     *       25
     *     7
     *       8
     * 9
     */
    std::vector<int> source{20, 21, 22, 23, 24, 25};

    auto it = this->sut.insert(
        std::ranges::find(this->sut, 5), DestinationPosition{1}, source);

    EXPECT_EQ(it, std::ranges::find(this->sut, 20));
    EXPECT_THAT(
        this->sut,
        ElementsAre(1, 2, 10, 3, 4, 5, 6, 20, 21, 22, 23, 24, 25, 7, 8, 9));
}

TYPED_TEST(GenericTreeFixture, batch_insert_at_optional_position)
{
    /*
     * 1
     *   2
     *     10
     *   3
     * 4
     *   5
     *     6
     *       20
     *       21
     *       22
     *       23
     *       24
     *       25
     *     7
     *       8
     * 9
     */
    std::vector<int> source{20, 21, 22, 23, 24, 25};
    std::optional<DestinationPosition> maybe_pos{1};

    auto it =
        this->sut.insert(std::ranges::find(this->sut, 5), maybe_pos, source);

    EXPECT_EQ(it, std::ranges::find(this->sut, 20));
    EXPECT_THAT(
        this->sut,
        ElementsAre(1, 2, 10, 3, 4, 5, 6, 20, 21, 22, 23, 24, 25, 7, 8, 9));
}

TYPED_TEST(GenericTreeFixture,
           batch_insert_throws_when_destination_out_of_range)
{
    std::vector<int> source{20, 21, 22, 23, 24, 25};

    EXPECT_THROW(this->sut.insert(std::ranges::find(this->sut, 5),
                                  DestinationPosition{700000},
                                  source),
                 std::out_of_range);
}

TYPED_TEST(GenericTreeFixture,
           move_throws_when_source_count_or_destination_out_of_range)
{
    auto tree = TestFixture::make_multiroot_sample_tree();
    auto source_parent = std::ranges::find(tree, 2);
    auto destination_parent = std::ranges::find(tree, 5);

    EXPECT_THROW(tree.move_nodes(source_parent,
                                 SourcePosition{0},
                                 Count{2},
                                 destination_parent,
                                 DestinationPosition{0}),
                 std::out_of_range);
    EXPECT_THROW(tree.move_nodes(source_parent,
                                 SourcePosition{2},
                                 Count{1},
                                 destination_parent,
                                 DestinationPosition{0}),
                 std::out_of_range);
    EXPECT_THROW(tree.move_nodes(source_parent,
                                 SourcePosition{0},
                                 Count{1},
                                 destination_parent,
                                 DestinationPosition{3}),
                 std::out_of_range);
}

TYPED_TEST(GenericTreeFixture,
           move_overload_throws_when_source_count_or_destination_out_of_range)
{
    auto tree = TestFixture::make_multiroot_sample_tree();
    auto source_parent = std::ranges::find(tree, 2);
    auto destination_parent = source_parent;

    EXPECT_THROW(tree.move_nodes(source_parent,
                                 SourcePosition{1},
                                 Count{1},
                                 destination_parent,
                                 DestinationPosition{0}),
                 std::out_of_range);
    EXPECT_THROW(tree.move_nodes(source_parent,
                                 SourcePosition{0},
                                 Count{2},
                                 destination_parent,
                                 DestinationPosition{0}),
                 std::out_of_range);
    EXPECT_THROW(tree.move_nodes(source_parent,
                                 SourcePosition{0},
                                 Count{1},
                                 destination_parent,
                                 DestinationPosition{2}),
                 std::out_of_range);
}

TYPED_TEST(GenericTreeFixture, move_subtree_to_itself)
{
    auto tree = TestFixture::make_multiroot_sample_tree();
    auto source_parent = std::ranges::find(tree, 2);
    auto destination_parent = source_parent;

    tree.move_nodes(source_parent,
                    SourcePosition{0},
                    Count{1},
                    destination_parent,
                    DestinationPosition{0});

    EXPECT_EQ(TestFixture::make_multiroot_sample_tree(), tree);
}

TYPED_TEST(GenericTreeFixture, moving_nodes_between_leaves)
{
    auto tree = TestFixture::make_multiroot_sample_tree();
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

TYPED_TEST(GenericTreeFixture, moving_nodes_up_within_same_parent)
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
    typename TestFixture::IntTree tree;
    tree.insert(tree.end(), 1);
    tree.insert(tree.end(), 2);
    tree.insert(tree.end(), 3);
    tree.insert(tree.end(), 4);
    typename TestFixture::IntTree expected;
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

TYPED_TEST(GenericTreeFixture, moving_nodes_down_within_same_parent)
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
    typename TestFixture::IntTree tree;
    tree.insert(tree.end(), 1);
    tree.insert(tree.end(), 2);
    tree.insert(tree.end(), 3);
    tree.insert(tree.end(), 4);
    typename TestFixture::IntTree expected;
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

TYPED_TEST(GenericTreeFixture,
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

    typename TestFixture::IntTree tree;
    tree.insert(tree.end(), 1);
    tree.insert(tree.end(), 2);
    tree.insert(tree.end(), 3);
    tree.insert(tree.end(), 4);
    typename TestFixture::IntTree expected;
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

TYPED_TEST(GenericTreeFixture,
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

    typename TestFixture::IntTree tree;
    tree.insert(tree.end(), 1);
    tree.insert(tree.end(), 2);
    tree.insert(tree.end(), 3);
    tree.insert(tree.end(), 4);
    typename TestFixture::IntTree expected;
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

TYPED_TEST(GenericTreeFixture,
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

    typename TestFixture::IntTree tree;
    tree.insert(tree.end(), 1);
    tree.insert(tree.end(), 2);
    tree.insert(tree.end(), 3);
    tree.insert(tree.end(), 4);
    typename TestFixture::IntTree expected;
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

TYPED_TEST(GenericTreeFixture, moving_nodes_in_reparenting_combinations)
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
    typename TestFixture::IntTree tree;
    tree.insert(tree.end(), 1);
    tree.insert(tree.end(), 2);
    tree.insert(tree.end(), 3);
    tree.insert(tree.end(), 4);
    typename TestFixture::IntTree expected;
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

TYPED_TEST(GenericTreeFixture, moving_nodes_between_roots_from_left_to_mid)
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

    auto tree = TestFixture::make_move_testing_tree();
    auto source_parent = std::ranges::find(tree, 1);
    auto destination_parent = std::ranges::find(tree, 6);

    tree.move_nodes(source_parent,
                    SourcePosition{0},
                    Count{2},
                    destination_parent,
                    DestinationPosition{2});

    EXPECT_THAT(tree,
                ::testing::ElementsAre(1, 4, 5, 6, 7, 8, 2, 3, 10, 11, 9, 12));
}

TYPED_TEST(GenericTreeFixture, moving_nodes_between_roots_from_right_to_left)
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
    auto tree = TestFixture::make_move_testing_tree();
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

TYPED_TEST(GenericTreeFixture, moving_nodes_between_roots_from_mid_to_right)
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
    auto tree = TestFixture::make_move_testing_tree();
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

TYPED_TEST(GenericTreeFixture, flatten_tree)
{
    const auto flattened = this->sut.flatten();

    EXPECT_THAT(flattened,
                ElementsAre(std::nullopt,
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

TYPED_TEST(GenericTreeFixture, transform_subtree)
{
    const auto subtree =
        this->sut.transform(std::ranges::find(this->sut, 5),
                            [](auto val) { return std::to_string(val); });

    EXPECT_THAT(subtree, ElementsAre("5", "6", "7", "8"));
}

TYPED_TEST(GenericTreeFixture, insert_with_projection)
{
    std::vector<std::pair<std::string, int>> source{
        {"11", 11}, {"12", 12}, {"77", 77}};

    this->sut.insert(std::ranges::find(this->sut, 5),
                     DestinationPosition{1},
                     begin(source),
                     end(source),
                     [](auto& p) { return p.second; });

    EXPECT_THAT(this->sut,
                ElementsAre(1, 2, 10, 3, 4, 5, 6, 11, 12, 77, 7, 8, 9));
}

TYPED_TEST(GenericTreeFixture, insert_range)
{

    std::vector<std::pair<std::string, int>> source{
        {"11", 11}, {"12", 12}, {"77", 77}};

    this->sut.insert(std::ranges::find(this->sut, 5),
                     DestinationPosition{1},
                     source,
                     [](auto& p) { return p.second; });

    EXPECT_THAT(this->sut,
                ElementsAre(1, 2, 10, 3, 4, 5, 6, 11, 12, 77, 7, 8, 9));
}

TYPED_TEST(GenericTreeFixture, take_subtree)
{
    auto tree = TestFixture::make_move_testing_tree();

    const auto subtree = tree.take_subtree(std::ranges::find(tree, 3));

    EXPECT_THAT(tree, ::testing::ElementsAre(1, 2, 4, 5, 6, 7, 8, 9, 12));
    EXPECT_THAT(subtree, ::testing::ElementsAre(3, 10, 11));
}

TYPED_TEST(GenericTreeFixture, take_and_reinsert_subtree)
{
    auto initial = this->sut;
    auto subtree = this->sut.take_subtree(std::ranges::find(this->sut, 4));

    this->sut.insert_subtree(this->sut.end(), subtree, DestinationPosition{1});

    EXPECT_EQ(initial, this->sut);
}

TYPED_TEST(GenericTreeFixture, insert_empty_subtree)
{
    typename TestFixture::IntTree subtree;

    this->sut.insert_subtree(
        std::ranges::find(this->sut, 5), subtree, DestinationPosition{1});

    EXPECT_THAT(this->sut, ElementsAre(1, 2, 10, 3, 4, 5, 6, 7, 8, 9));
}

TYPED_TEST(GenericTreeFixture, insert_subtree)
{
    typename TestFixture::IntTree subtree;
    auto first = subtree.insert(subtree.end(), 101);
    auto second = subtree.insert(first, 102);
    subtree.insert(second, 103);
    subtree.insert(second, 104);

    this->sut.insert_subtree(
        std::ranges::find(this->sut, 5), subtree, DestinationPosition{1});

    EXPECT_THAT(this->sut,
                ElementsAre(1, 2, 10, 3, 4, 5, 6, 101, 102, 103, 104, 7, 8, 9));
}

TYPED_TEST(GenericTreeFixture, insert_subtree_at_root)
{
    typename TestFixture::IntTree subtree;
    auto first = subtree.insert(subtree.end(), 101);
    auto second = subtree.insert(first, 102);
    subtree.insert(second, 103);
    subtree.insert(second, 104);

    this->sut.insert_subtree(this->sut.end(), subtree, DestinationPosition{1});

    EXPECT_THAT(this->sut,
                ElementsAre(1, 2, 10, 3, 101, 102, 103, 104, 4, 5, 6, 7, 8, 9));
}

TYPED_TEST(GenericTreeFixture, insert_subtree_optional_overload)
{
    typename TestFixture::IntTree subtree;
    auto first = subtree.insert(subtree.end(), 101);
    auto second = subtree.insert(first, 102);
    subtree.insert(second, 103);
    subtree.insert(second, 104);

    this->sut.insert_subtree(
        std::ranges::find(this->sut, 5), subtree, std::nullopt);

    EXPECT_THAT(this->sut,
                ElementsAre(1, 2, 10, 3, 4, 5, 6, 7, 8, 101, 102, 103, 104, 9));
}

TYPED_TEST(GenericTreeFixture, copy_constructor)
{
    auto copy{this->sut};

    EXPECT_EQ(copy, this->sut);
}

TYPED_TEST(GenericTreeFixture, returns_children_iterators)
{
    auto it = std::ranges::find(this->sut, 5);
    std::vector<int> actual;
    std::vector<int> expected{6, 7};

    auto view = this->sut.children_iterators(it) |
                std::views::transform([](auto iter) { return *iter; });
    std::ranges::copy(view, std::back_inserter(actual));

    EXPECT_EQ(expected, actual);
}

TYPED_TEST(GenericTreeFixture, const_children_iterators)
{
    const auto tree = this->sut;

    std::vector<int> actual;
    const auto it = std::ranges::find(tree, 5);
    std::ranges::copy(
        std::views::reverse(tree.children_iterators(it)) |
            std::views::transform([](auto iter) { return *iter; }),
        std::back_inserter(actual));

    EXPECT_THAT(actual, ElementsAre(7, 6));
}

TYPED_TEST(GenericTreeFixture, returns_subtree)
{
    typename TestFixture::IntTree expected;
    auto it1 = expected.insert(expected.end(), 5);
    expected.insert(it1, 6);
    auto it2 = expected.insert(it1, 7);
    expected.insert(it2, 8);

    const auto actual = this->sut.subtree(std::ranges::find(this->sut, 5));

    EXPECT_EQ(expected, actual);
}

TYPED_TEST(GenericTreeFixture, arranges_tree_by_predicate)
{
    /* Initial tree
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

    /* Rearranged tree
     *  2
     *    10
     *  10
     *  4
     *    5
     *      6
     *      7
     *        8
     *  6
     *  8
     */
    typename TestFixture::IntTree expected;
    auto predicate = [](int x) { return x % 2 == 0; };
    auto it1 = expected.insert(expected.end(), 2);
    expected.insert(it1, 10);
    expected.insert(expected.end(), 10);
    auto it2 = expected.insert(expected.end(), 4);
    auto it3 = expected.insert(it2, 5);
    expected.insert(it3, 6);
    auto it4 = expected.insert(it3, 7);
    expected.insert(it4, 8);
    expected.insert(expected.end(), 6);
    expected.insert(expected.end(), 8);

    const auto actual = this->sut.arrange_by(predicate);

    EXPECT_EQ(expected, actual);
}

TYPED_TEST(GenericTreeFixture, finds_value_in_subtree)
{
    /*
     *  1
     *    2
     *    3
     *      2
     *        1
     *    2
     *  2
     */
    typename TestFixture::IntTree tree;
    auto it1 = tree.insert(tree.end(), 1);
    tree.insert(it1, 2);
    auto subtree_it = tree.insert(it1, 3);
    auto it3 = tree.insert(subtree_it, 2);
    tree.insert(it3, 1);
    tree.insert(it1, 2);
    tree.insert(tree.end(), 2);
    auto actual_it = tree.find(subtree_it, 2);

    EXPECT_EQ(2, *actual_it);
    EXPECT_EQ(3, std::distance(tree.begin(), actual_it));
    EXPECT_EQ(tree.end(), tree.find(subtree_it, 77));
}

TYPED_TEST(GenericTreeFixture, finds_projected_value_in_subtree)
{
    /*
     * 1 "1"
     *    2 "1"
     *    3 "1"
     *       2 "2"
     *         1 "2"
     *    2 "3"
     * 2 "4"
     */
    typename TestFixture::CompoundTree tree;
    auto it1 = tree.insert(tree.end(), CompoundType{1, "1"});
    tree.insert(it1, CompoundType{2, "1"});
    auto subtree_it = tree.insert(it1, CompoundType{3, "1"});
    auto it3 = tree.insert(subtree_it, CompoundType{2, "2"});
    tree.insert(it3, CompoundType{1, "2"});
    tree.insert(it1, CompoundType{2, "3"});
    tree.insert(tree.end(), CompoundType{2, "4"});

    auto projection = [](const auto& elem) { return elem.some_value; };
    auto actual_it = tree.find(subtree_it, 2, projection);

    EXPECT_EQ(2, actual_it->some_value);
    EXPECT_EQ("2", actual_it->id);
    EXPECT_EQ(tree.end(), tree.find(subtree_it, 77, projection));
}

TYPED_TEST(GenericTreeFixture, finds_value_satisfying_predicate_in_subtree)
{
    /*
     * 1 "1"
     *    2 "1"
     *    3 "1"
     *       2 "2"
     *         1 "2"
     *    2 "3"
     * 2 "4"
     */
    typename TestFixture::CompoundTree tree;
    auto it1 = tree.insert(tree.end(), CompoundType{1, "1"});
    tree.insert(it1, CompoundType{2, "1"});
    auto subtree_it = tree.insert(it1, CompoundType{3, "1"});
    auto it3 = tree.insert(subtree_it, CompoundType{2, "2"});
    tree.insert(it3, CompoundType{1, "2"});
    tree.insert(it1, CompoundType{2, "3"});
    tree.insert(tree.end(), CompoundType{2, "4"});

    auto predicate = [](const auto& elem) { return elem.some_value == 2; };
    auto actual_it = tree.find_if(subtree_it, predicate);

    EXPECT_EQ(2, actual_it->some_value);
    EXPECT_EQ("2", actual_it->id);
    EXPECT_EQ(tree.end(), tree.find_if(subtree_it, [](const auto& el) {
        return el.some_value == 77;
    }));
}

TYPED_TEST(GenericTreeFixture,
           finds_projected_value_satisfying_predicate_in_subtree)
{
    /*
     * 1 "1"
     *    2 "1"
     *    3 "1"
     *       2 "222"
     *         1 "2"
     *    2 "3"
     * 2 "4"
     */
    typename TestFixture::CompoundTree tree;
    auto it1 = tree.insert(tree.end(), CompoundType{1, "1"});
    tree.insert(it1, CompoundType{2, "1"});
    auto subtree_it = tree.insert(it1, CompoundType{3, "1"});
    auto it3 = tree.insert(subtree_it, CompoundType{2, "222"});
    tree.insert(it3, CompoundType{1, "2"});
    tree.insert(it1, CompoundType{2, "3"});
    tree.insert(tree.end(), CompoundType{2, "4"});

    auto projection = [](const auto& elem) { return elem.id; };
    auto predicate = [](const auto& str) { return str.size() >= 2; };
    auto actual_it = tree.find_if(subtree_it, predicate, projection);

    EXPECT_EQ(2, actual_it->some_value);
    EXPECT_EQ("222", actual_it->id);
    EXPECT_EQ(tree.end(),
              tree.find_if(
                  subtree_it,
                  [](const auto& str) { return str.size() == 77; },
                  projection));
}
