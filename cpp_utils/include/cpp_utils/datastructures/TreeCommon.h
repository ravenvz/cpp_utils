#ifndef TREECOMMON_H_HPODLZ4K
#define TREECOMMON_H_HPODLZ4K

#include "cpp_utils/types/NamedType.h"
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <functional>
#include <iterator>
#include <queue>
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

/* Tree types do provide iterators, but those iterators do not support iterating
 * only over some subtree. I.e. suppose we have some tree:
 *
 * 1
 *   2
 *     3
 * 4
 *   5
 *   6
 *   7
 *     8
 *
 * Suppose we need to do some iterator-based operation like find, etc. over a
 * subtree rooted at 2. We can do this with default iterator of tree type, but
 * we need to know tree structure for that, because if we do not stop iteration
 * after processing node 3, default iterator would proceed to node 4 and so on.
 *
 * Functions below do provide ability to perform iterator-based operations only
 * over a subtree given iterator pointing to the root of this subtree (only
 * nodes 2 and 3 will be processed in our example and iteration will stop).
 *
 * If original tree root is given as a subtree root (tree.end() iterator),
 * these functions typically default to corresponding std::ranges function.
 *
 * Those functions named as their std::ranges equivalents and have similar
 * signatures, but they do take a tree itself as an additional parameter.
 */

template <typename TreeType,
          typename Proj = std::identity,
          std::indirectly_unary_invocable<
              std::projected<typename TreeType::iterator, Proj>> Fun>
auto for_each(TreeType& tree,
              typename TreeType::iterator subtree_root,
              Fun fn,
              Proj proj = {}) -> void
{
    if (subtree_root == tree.end()) {
        std::ranges::for_each(tree, fn, proj);
        return;
    }

    std::stack<typename TreeType::iterator> frontier;
    frontier.push(subtree_root);

    while (not frontier.empty()) {
        auto current = frontier.top();
        frontier.pop();

        std::invoke(fn, std::invoke(proj, *current));

#ifdef __cpp_lib_containers_ranges
        frontier.push_range(tree.children_iterators(current) |
                            std::views::reverse);
#else
        std::ranges::for_each(
            tree.children_iterators(current) | std::views::reverse,
            [&frontier](auto child) { frontier.push(child); });
#endif
    }
}

template <typename TreeType,
          typename Proj = std::identity,
          std::indirectly_unary_invocable<
              std::projected<typename TreeType::const_iterator, Proj>> Fun>
auto for_each(const TreeType& tree,
              typename TreeType::const_iterator subtree_root,
              Fun fn,
              Proj proj = {}) -> void
{
    if (subtree_root == tree.cend()) {
        std::ranges::for_each(tree, fn, proj);
        return;
    }

    std::stack<typename TreeType::const_iterator> frontier;
    frontier.push(subtree_root);

    while (not frontier.empty()) {
        auto current = frontier.top();
        frontier.pop();

        std::invoke(fn, std::invoke(proj, *current));

#ifdef __cpp_lib_containers_ranges
        frontier.push_range(tree.children_iterators(current) |
                            std::views::reverse);
#else
        std::ranges::for_each(
            tree.children_iterators(current) | std::views::reverse,
            [&frontier](auto child) { frontier.push(child); });
#endif
    }
}

// There is no mutable overload for this function as it is not a good idea to
// expose possibility to use tree iteratora operation while iterating the tree.
template <typename TreeType, typename Fun>
auto for_each_it(const TreeType& tree,
                 typename TreeType::const_iterator subtree_root,
                 Fun fn) -> void
{
    if (subtree_root == tree.cend()) {
        // std::ranges::for_each(tree, fn, proj);
        // return;
        for (auto it = tree.cbegin(); it != tree.cend(); ++it) {
            std::invoke(fn, it);
        }
        return;
    }

    std::stack<typename TreeType::const_iterator> frontier;
    frontier.push(subtree_root);

    while (not frontier.empty()) {
        auto current = frontier.top();
        frontier.pop();

        std::invoke(fn, current);

#ifdef __cpp_lib_containers_ranges
        frontier.push_range(tree.children_iterators(current) |
                            std::views::reverse);
#else
        std::ranges::for_each(
            tree.children_iterators(current) | std::views::reverse,
            [&frontier](auto child) { frontier.push(child); });
#endif
    }
}
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
auto find_if(const TreeType& tree,
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

/* Returns new tree that has only nodes that satisfies predicate, others are
 * pruned with all children.
 *
 * Precidate takes tree payload. For more complex filtering tasks filter_it
 * function is provided.
 * */
template <typename TreeType>
auto filter(
    const TreeType& tree,
    std::indirect_unary_predicate<typename TreeType::const_iterator> auto pred)
    -> TreeType
{
    TreeType res;
    std::queue<std::pair<typename TreeType::const_iterator,
                         typename TreeType::iterator>>
        frontier;
    for (const auto child : tree.children_iterators(tree.end())) {
        if (not pred(*child)) {
            continue;
        }
        auto res_it = res.insert(res.end(), *child);
        frontier.push({child, res_it});
    }

    while (not frontier.empty()) {
        auto [current, res_it] = frontier.front();
        frontier.pop();

        for (auto child : tree.children_iterators(current)) {
            if (not pred(*child)) {
                continue;
            }
            auto it = res.insert(res_it, *child);
            frontier.push({child, it});
        }
    }

    return res;
}

/* Returns new tree that has only nodes that satisfies predicate, others are
 * pruned with all children.
 *
 * Predicate task const_iterator instead of tree payload. Might be useful for
 * more complex filtering tasks that are using information only functions taking
 * iterator can provide, i.e. information about parent-children relationships.
 * */
template <typename TreeType>
auto filter_it(const TreeType& tree,
               std::predicate<typename TreeType::const_iterator> auto pred)
    -> TreeType
{
    TreeType res;
    std::queue<std::pair<typename TreeType::const_iterator,
                         typename TreeType::iterator>>
        frontier;
    for (const auto child : tree.children_iterators(tree.end())) {
        if (not pred(child)) {
            continue;
        }
        auto res_it = res.insert(res.end(), *child);
        frontier.push({child, res_it});
    }

    while (not frontier.empty()) {
        auto [current, res_it] = frontier.front();
        frontier.pop();

        for (auto child : tree.children_iterators(current)) {
            if (not pred(child)) {
                continue;
            }
            auto it = res.insert(res_it, *child);
            frontier.push({child, it});
        }
    }

    return res;
}

} // namespace ds

#endif /* end of include guard: TREECOMMON_H_HPODLZ4K */
