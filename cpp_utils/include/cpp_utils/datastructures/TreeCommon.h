#ifndef TREECOMMON_H_HPODLZ4K
#define TREECOMMON_H_HPODLZ4K

#include "cpp_utils/types/NamedType.h"
#include <concepts>
#include <cstdint>
#include <functional>
#include <iterator>
#include <stack>

namespace details {

struct CountTag { };

struct SourcePosTag { };

struct DestinationPosTag { };

} // namespace details

namespace ds {

using Count = types::ImplicitNamedType<int64_t, details::CountTag>;

using SourcePosition = types::ImplicitNamedType<int64_t, details::SourcePosTag>;

using DestinationPosition =
    types::ImplicitNamedType<int64_t, details::DestinationPosTag>;

// Search for value in the subtree.
template <typename TreeType, typename Proj = std::identity, typename V>
    requires std::indirect_binary_predicate<
                 std::ranges::equal_to,
                 std::projected<typename TreeType::iterator, Proj>,
                 const V*>
auto find(TreeType& tree,
          typename TreeType::iterator subtree_root,
          const V& value,
          Proj proj = {}) -> TreeType::iterator
{
    if (subtree_root == tree.end()) {
        // Since we search all tree in this case, we could just fallback to
        // more generic algorithm
        return std::ranges::find(tree, value, proj);
    }
    std::stack<typename TreeType::iterator> frontier;
    frontier.push(subtree_root);

    while (not frontier.empty()) {
        auto current = frontier.top();
        frontier.pop();

        for (auto child : tree.children_iterators(current)) {
            if (std::invoke(proj, *child) == value) {
                return child;
            }
            frontier.push(child);
        }
    }

    return tree.end();
}

// Search for value in the subtree.
template <typename TreeType, typename Proj = std::identity, typename V>
    requires std::indirect_binary_predicate<
                 std::ranges::equal_to,
                 std::projected<typename TreeType::const_iterator, Proj>,
                 const V*>
auto find(const TreeType& tree,
          typename TreeType::const_iterator subtree_root,
          const V& value,
          Proj proj = {}) -> TreeType::const_iterator
{
    if (subtree_root == tree.end()) {
        // Since we search all tree in this case, we could just fallback to
        // more generic algorithm
        return std::ranges::find(tree, value, proj);
    }
    std::stack<typename TreeType::const_iterator> frontier;
    frontier.push(subtree_root);

    while (not frontier.empty()) {
        auto current = frontier.top();
        frontier.pop();

        for (auto child : tree.children_iterators(current)) {
            if (std::invoke(proj, *child) == value) {
                return child;
            }
            frontier.push(child);
        }
    }

    return tree.cend();
}

template <typename TreeType,
          typename Proj = std::identity,
          std::indirect_unary_predicate<
              std::projected<typename TreeType::iterator, Proj>> Pred>
auto find_if(TreeType& tree,
             typename TreeType::iterator subtree_root,
             Pred pred,
             Proj proj = {}) -> TreeType::iterator
{
    if (subtree_root == tree.end()) {
        // Since we search all tree in this case, we could just fallback to
        // more generic algorithm
        return std::ranges::find_if(tree, pred, proj);
    }

    std::stack<typename TreeType::iterator> frontier;
    frontier.push(subtree_root);

    while (not frontier.empty()) {
        auto current = frontier.top();
        frontier.pop();

        for (auto child : tree.children_iterators(current)) {
            if (std::invoke(pred, std::invoke(proj, *child))) {
                return child;
            }
            frontier.push(child);
        }
    }

    return tree.end();
}

template <typename TreeType,
          typename Proj = std::identity,
          std::indirect_unary_predicate<
              std::projected<typename TreeType::const_iterator, Proj>> Pred>
auto find_if(TreeType& tree,
             typename TreeType::const_iterator subtree_root,
             Pred pred,
             Proj proj = {}) -> TreeType::const_iterator
{
    if (subtree_root == tree.end()) {
        // Since we search all tree in this case, we could just fallback to
        // more generic algorithm
        return std::ranges::find_if(tree, pred, proj);
    }

    std::stack<typename TreeType::const_iterator> frontier;
    frontier.push(subtree_root);

    while (not frontier.empty()) {
        auto current = frontier.top();
        frontier.pop();

        for (auto child : tree.children_iterators(current)) {
            if (std::invoke(pred, std::invoke(proj, *child))) {
                return child;
            }
            frontier.push(child);
        }
    }

    return tree.cend();
}

// Returns tree that contains all subtrees with roots satisfying given
// predicate. All those subtrees will be children of new tree root.
//
// Note that if in some subtree there are subtree or node satisfying given
// predicate, it will be also added to the result tree (therefore some
// values will be duplicated).
template <typename TreeType>
auto arrange_by(
    const TreeType& tree,
    std::predicate<typename TreeType::const_iterator::element_type> auto pred)
    -> TreeType
{
    TreeType res;
    for (auto it = tree.cbegin(); it != tree.cend(); ++it) {
        if (std::invoke(pred, *it)) {
            res.insert_subtree(res.end(),
                               tree.subtree(it),
                               DestinationPosition{static_cast<int>(
                                   res.children(res.end()).size())});
        }
    }
    return res;
}

} // namespace ds

#endif /* end of include guard: TREECOMMON_H_HPODLZ4K */
