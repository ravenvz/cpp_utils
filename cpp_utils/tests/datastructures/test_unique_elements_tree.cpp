#include "datastructures/UniqueElementsTree.h"
#include "gtest/gtest.h"

#include <bits/iterator_concepts.h>
#include <functional>

namespace {

struct CompoundType {
    std::string id;
    int payload{0};

    friend bool operator==(const CompoundType&, const CompoundType&) = default;
};

template <class CharT, class Traits>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const CompoundType& ct)
{
    os << "CompoundType{id = " << ct.id << ", payload = " << ct.payload << "}";
    return os;
}

struct Selector {
    auto operator()(const CompoundType& node) const -> std::string
    {
        return node.id;
    }

    friend bool operator==(const Selector&, const Selector&) = default;
};

auto make_sample_tree() -> ds::UniqueElementsTree<CompoundType, Selector>
{
    ds::UniqueElementsTree<CompoundType, Selector> tree;
    /*
     * 1
     *   2
     *     10
     *   3
     * 4
     *   5
     *     7
     *       8
     *         6
     * 9
     */
    tree.add_child(CompoundType{"1", 1});
    tree.add_child(CompoundType{"2", 2}, "1");
    tree.add_child(CompoundType{"10", 10}, "2");
    tree.add_child(CompoundType{"3", 3}, "1");
    tree.add_child(CompoundType{"4", 4});
    tree.add_child(CompoundType{"5", 5}, "4");
    tree.add_child(CompoundType{"7", 7}, "5");
    tree.add_child(CompoundType{"8", 8}, "7");
    tree.add_child(CompoundType{"6", 6}, "8");
    tree.add_child(CompoundType{"9", 9});

    return tree;
}

// template <typename PayloadT, typename SelectorT>
// auto operator==(const ds::UniqueElementsTree<PayloadT, SelectorT>& lhs,
//                 const ds::UniqueElementsTree<PayloadT, SelectorT>& rhs) ->
//                 bool
// {
//
//     return false;
// }

} // namespace

class UniqueElementsTreeFixture : public ::testing::Test {
public:
    ds::UniqueElementsTree<CompoundType, Selector> sut{make_sample_tree()};
};

TEST_F(UniqueElementsTreeFixture, throws_when_unique_key_constraint_violated)
{
    ds::UniqueElementsTree<int> tree;
    tree.add_child(1);
    tree.add_child(2);

    EXPECT_THROW(tree.add_child(1), ds::UniqueKeyError);
}

TEST_F(UniqueElementsTreeFixture,
       throws_when_adding_node_with_parent_that_is_not_in_tree)
{
    ds::UniqueElementsTree<int> tree;
    tree.add_child(1);

    EXPECT_THROW(tree.add_child(2, 7), ds::KeyError);
}

// TEST_F(UniqueElementsTreeFixture, mapping_tree_returns_mapped_tree)
// {
//     const auto tree = make_sample_tree();
//
//     ds::UniqueElementsTree<int> t;
//     t.add_child(1);
//     t.add_child(2);
//     t.add_child(3, 1);
//
//     // const auto actual = tree.map();
// }
//
TEST_F(UniqueElementsTreeFixture, flatten_tree)
{
    const auto tree = make_sample_tree();
    const std::vector<std::optional<CompoundType>> expected{
        std::nullopt,           std::nullopt,         CompoundType{"1", 1},
        CompoundType{"4", 4},   CompoundType{"9", 9}, std::nullopt,
        CompoundType{"2", 2},   CompoundType{"3", 3}, std::nullopt,
        CompoundType{"5", 5},   std::nullopt,         std::nullopt,
        CompoundType{"10", 10}, std::nullopt,         std::nullopt,
        CompoundType{"7", 7},   std::nullopt,         std::nullopt,
        CompoundType{"8", 8},   std::nullopt,         CompoundType{"6", 6},
        std::nullopt,           std::nullopt};

    const auto flattened = tree.flatten();

    EXPECT_EQ(expected, flattened);
}

TEST_F(UniqueElementsTreeFixture, unflatten_tree)
{
    const std::vector<std::optional<CompoundType>> flattened{
        CompoundType{},         std::nullopt,         CompoundType{"1", 1},
        CompoundType{"4", 4},   CompoundType{"9", 9}, std::nullopt,
        CompoundType{"2", 2},   CompoundType{"3", 3}, std::nullopt,
        CompoundType{"5", 5},   std::nullopt,         std::nullopt,
        CompoundType{"10", 10}, std::nullopt,         std::nullopt,
        CompoundType{"7", 7},   std::nullopt,         std::nullopt,
        CompoundType{"8", 8},   std::nullopt,         CompoundType{"6", 6},
        std::nullopt,           std::nullopt};

    const auto restored_tree =
        ds::UniqueElementsTree<CompoundType, Selector>::unflatten(flattened);

    std::cout << sut.to_string() << std::endl;
    std::cout << std::endl;
    std::cout << restored_tree.to_string() << std::endl;

    EXPECT_EQ(sut, restored_tree);
}

TEST_F(UniqueElementsTreeFixture, flatten_and_unflatten)
{
    auto tree = make_sample_tree();
    auto flattened_tree = tree.flatten();
    auto restored_tree =
        ds::UniqueElementsTree<CompoundType, Selector>::unflatten(
            flattened_tree);

    EXPECT_EQ(tree, restored_tree);
}

TEST_F(UniqueElementsTreeFixture, const_dfs_iterator)
{
    auto tree = make_sample_tree();
    static_assert(std::forward_iterator<decltype(tree.cbegin_dfs())>);
    const std::vector<CompoundType> expected{{"1", 1},
                                             {"2", 2},
                                             {"10", 10},
                                             {"3", 3},
                                             {"4", 4},
                                             {"5", 5},
                                             {"7", 7},
                                             {"8", 8},
                                             {"6", 6},
                                             {"9", 9}};

    std::vector<CompoundType> dfs_order;
    for (auto it = tree.cbegin_dfs(); it != tree.cend_dfs(); ++it) {
        dfs_order.push_back(*it);
    }

    EXPECT_EQ(expected, dfs_order);
}

TEST_F(UniqueElementsTreeFixture, dfs_iterator)
{
    auto tree = make_sample_tree();
    static_assert(std::forward_iterator<decltype(tree.begin_dfs())>);
    const std::vector<CompoundType> expected{{"1", 2},
                                             {"2", 4},
                                             {"10", 20},
                                             {"3", 6},
                                             {"4", 8},
                                             {"5", 10},
                                             {"7", 14},
                                             {"8", 16},
                                             {"6", 12},
                                             {"9", 18}};

    std::for_each(tree.begin_dfs(), tree.end_dfs(), [](auto& elem) {
        elem.payload *= 2;
    });
    std::vector<CompoundType> dfs_order;
    std::transform(cbegin_dfs(tree),
                   cend_dfs(tree),
                   std::back_inserter(dfs_order),
                   [](const auto& elem) { return elem; });

    EXPECT_EQ(expected, dfs_order);
}
