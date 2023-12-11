#ifndef IMMUTABLETREE_H_AAE9JBHV
#define IMMUTABLETREE_H_AAE9JBHV

#include "algorithms/alg_ext.h"
#include "datastructures/TreeCommon.h"
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <iterator>
#include <optional>
#include <queue>
#include <ranges>
#include <span>
#include <sstream>
#include <stack>
#include <vector>

namespace ds {

template <std::default_initializable T> class ImmutableTree {
private:
    struct Node {
        int64_t parent{-1};
        T payload{};
        int64_t pos{0};
        std::vector<int64_t> children{};
    };

public:
    template <typename TransformFunc>
    using TransformResultT =
        std::remove_cvref_t<std::invoke_result_t<TransformFunc, const T&>>;

    template <class v_type, class n_type> class DfsIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using element_type = v_type;

        friend class DfsIterator<const v_type, const n_type>;
        friend class ImmutableTree;

        DfsIterator() = default;

        explicit DfsIterator(int64_t p_, std::span<n_type> data_ref)
            : ptr{p_}
            , data{data_ref}
        {
        }

        template <class other_n_type, class other_v_type>
            requires(!std::is_const_v<other_n_type> &&
                     std::is_const_v<n_type> &&
                     std::is_same_v<std::remove_const_t<n_type>, other_n_type>)
        DfsIterator(const DfsIterator<other_v_type, other_n_type>& rhs)
            : ptr{rhs.ptr}
            , prev{rhs.prev}
        {
        }

        auto operator*() const -> element_type&
        {
            return data[static_cast<size_t>(ptr)].payload;
        }

        auto operator->() -> element_type*
        {
            return &data[static_cast<size_t>(ptr)].payload;
        }

        auto operator++() -> DfsIterator&
        {
            const Node& ptr_node = data[static_cast<size_t>(ptr)];
            const Node& prev_node = data[static_cast<size_t>(prev)];
            auto tmp = ptr;
            const bool bottom_reached{
                ptr_node.children.empty() or
                (prev != ptr_node.parent and
                 prev_node.pos + 1 >= std::ssize(ptr_node.children))};

            auto offset = [&]() {
                return static_cast<size_t>(
                    prev == ptr_node.parent ? 0 : prev_node.pos + 1);
            };

            ptr =
                bottom_reached ? ptr_node.parent : ptr_node.children[offset()];

            prev = tmp;

            if (ptr != -1 and prev != data[static_cast<size_t>(ptr)].parent) {
                this->operator++(1);
            }

            return *this;
        }

        auto operator++(int) -> DfsIterator
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        friend auto operator==(const DfsIterator& lhs, const DfsIterator& rhs)
            -> bool
        {
            return lhs.ptr == rhs.ptr;
        }

        friend auto operator!=(const DfsIterator& lhs, const DfsIterator& rhs)
            -> bool
        {
            return not(lhs == rhs);
        }

    private:
        int64_t ptr{0};
        int64_t prev{-1};
        std::span<n_type> data;
    };

    using value_type = T;
    using iterator = DfsIterator<value_type, Node>;
    using const_iterator = DfsIterator<const value_type, const Node>;

    ImmutableTree(std::ranges::input_range auto&& r)
        : ImmutableTree(std::ranges::begin(r), std::ranges::end(r))
    {
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
    ImmutableTree(I first, S last)
    {
        std::queue<iterator> frontier;
        frontier.push(begin());

        for (auto it = first + 2; not frontier.empty() and it != last; ++it) {
            auto parent_it{frontier.front()};
            frontier.pop();
            for (; it->has_value(); ++it) {
                auto child = insert(parent_it, std::forward<T>(**it));
                frontier.push(child);
            }
        }
    }

    ImmutableTree() = default;

    ImmutableTree(const ImmutableTree&) = default;

    auto operator=(const ImmutableTree&) -> ImmutableTree& = default;

    ImmutableTree(ImmutableTree&&) = default;

    auto operator=(ImmutableTree&&) -> ImmutableTree& = default;

    auto insert(iterator parent, T payload) -> iterator
    {
        const auto true_parent = find_true_index(parent);
        const auto pos{std::ssize(get_node(true_parent).children)};
        const auto child_index = insert_into_free_spot(
            Node{true_parent, std::move(payload), pos, std::vector<int64_t>{}});
        get_node(true_parent).children.push_back(child_index);
        return iterator{child_index, storage};
    }

    auto insert(iterator parent, T payload, DestinationPosition destination)
        -> iterator
    {
        const auto true_parent = find_true_index(parent);
        const auto child_index =
            insert_into_free_spot(Node{true_parent,
                                       std::move(payload),
                                       destination,
                                       std::vector<int64_t>()});
        auto& parent_children = get_node(true_parent).children;
        parent_children.insert(parent_children.begin() + destination,
                               child_index);
        fix_positions_and_parents(true_parent, destination);
        return iterator{child_index, storage};
    }

    template <std::ranges::input_range R, class Proj = std::identity>
    auto
    insert(iterator parent, DestinationPosition position, R&& r, Proj proj = {})
    {
        return insert(parent,
                      position,
                      std::ranges::begin(r),
                      std::ranges::end(r),
                      std::ref(proj));
    }

    template <std::input_iterator I,
              std::sentinel_for<I> S,
              class Proj = std::identity>
    auto insert(iterator parent,
                DestinationPosition position,
                I first,
                S last,
                Proj proj = {})
    {
        if (first == last) {
            return end();
        }
        const auto true_parent = find_true_index(parent);
        std::vector<int64_t> indexes(
            static_cast<size_t>(std::distance(first, last)));
        int64_t pos = position;
        std::transform(first, last, indexes.begin(), [&](auto&& source) {
            return insert_into_free_spot(Node{true_parent,
                                              std::invoke(proj, source),
                                              pos++,
                                              std::vector<int64_t>{}});
        });
        auto& parent_children = get_node(true_parent).children;

#ifdef __cpp_lib_containers_ranges
        parent_children.insert_range(parent_children.begin() + position,
                                     indexes);
#else
        parent_children.insert(
            parent_children.begin() + position, indexes.begin(), indexes.end());
#endif

        fix_positions_and_parents(true_parent, position);
        return iterator{indexes.front(), storage};
    }

    auto erase(iterator subtree) -> void
    {
        if (subtree == end()) {
            return;
        }
        const auto parent_index = find_true_index(parent(subtree));
        auto& parent_children = get_node(parent_index).children;
        parent_children.erase(parent_children.begin() +
                              get_node(subtree.ptr).pos);
        mark_removed(subtree.ptr);
        fix_positions_and_parents(parent_index, get_node(subtree.ptr).pos);
    }

    auto move_nodes(iterator source_parent,
                    SourcePosition source_pos,
                    Count count,
                    iterator destination_parent,
                    DestinationPosition destination_pos) -> void
    {
        const auto source_parent_index = find_true_index(source_parent);
        const auto destination_parent_index =
            find_true_index(destination_parent);

        auto& source_children = get_node(source_parent_index).children;
        auto& destination_children =
            get_node(destination_parent_index).children;

        if (source_parent_index == destination_parent_index) {
            alg::slide(source_children.begin() + source_pos,
                       source_children.begin() + source_pos + count,
                       source_children.begin() + destination_pos);
            fix_positions_and_parents(source_parent_index, 0);
            return;
        }

        auto [first, last] =
            alg::slide(source_children.begin() + source_pos,
                       source_children.begin() + source_pos + count,
                       source_children.end());
        source_children.erase(first, last);
        fix_positions_and_parents(source_parent_index, source_pos);

        destination_children.insert(
            destination_children.begin() + destination_pos, first, last);
        fix_positions_and_parents(destination_parent_index, destination_pos);
    }

    auto parent(const_iterator it) const -> const_iterator
    {
        if (get_node(it.ptr).parent == 0 or it == cend()) {
            return cend();
        }
        return const_iterator{get_node(it.ptr).parent, storage};
    }

    auto parent(iterator it) -> iterator
    {
        if (get_node(it.ptr).parent == 0 or it == end()) {
            return end();
        }
        return iterator{get_node(it.ptr).parent, storage};
    }

    auto children(const_iterator it)
    {
        const auto index = find_true_index(it);
        return std::views::transform(get_node(index).children,
                                     [this](const auto& child_id) {
                                         return get_node(child_id).payload;
                                     });
    }

    auto children(const_iterator it) const
    {
        const auto index = find_true_index(it);
        return std::views::transform(get_node(index).children,
                                     [this](const auto& child_id) {
                                         return get_node(child_id).payload;
                                     });
    }

    auto take_subtree(iterator subtree_root) -> ImmutableTree
    {
        auto tree = transform(subtree_root, std::identity{});
        erase(subtree_root);
        return tree;
    }

    auto position_in_children(const_iterator it) const -> int64_t
    {
        return it == cend() ? 0 : get_node(it.ptr).pos;
    }

    template <typename Func>
    auto transform(Func func) const -> ImmutableTree<TransformResultT<Func>>
    {
        return transform(cend(), func);
    }

    template <typename Func>
    auto transform(const_iterator subtree_root, Func func) const
        -> ImmutableTree<TransformResultT<Func>>
    {
        using Y = TransformResultT<Func>;
        ImmutableTree<Y> mapped;

        std::queue<std::pair<int64_t, typename ImmutableTree<Y>::iterator>>
            frontier;

        if (subtree_root == cend()) {
            const auto& children = get_node(0).children;

            for (auto child_id : children) {
                auto it = mapped.insert(mapped.end(),
                                        func(get_node(child_id).payload));
                frontier.push({child_id, it});
            }
        }
        else {
            auto it = mapped.insert(mapped.end(),
                                    func(get_node(subtree_root.ptr).payload));
            frontier.push({subtree_root.ptr, it});
        }

        while (not frontier.empty()) {
            auto [current, mapped_it] = frontier.front();
            frontier.pop();

            const auto& children = get_node(current).children;

            for (auto child_id : children) {
                auto it =
                    mapped.insert(mapped_it, func(get_node(child_id).payload));
                frontier.push({child_id, it});
            }
        }

        return mapped;
    }

    template <typename Func> auto map(iterator subtree_root, Func func) -> void
    {
        if (subtree_root == end()) {
            std::for_each(begin(), end(), func);
            return;
        }

        std::stack<int64_t> frontier;
        frontier.push(subtree_root.ptr);

        while (not frontier.empty()) {
            const auto current = frontier.top();
            frontier.pop();

            func(get_node(current).payload);

#ifdef __cpp_lib_containers_ranges
            frontier.push_range(get_node(current).children);
#else
            std::ranges::for_each(
                get_node(current).children,
                [&frontier](auto child) { frontier.push(child); });
#endif
        }
    }

    auto flatten() const -> std::vector<std::optional<T>>
    {
        std::queue<int64_t> frontier;
        frontier.push(0);

        std::vector<std::optional<T>> flattened{std::nullopt, std::nullopt};

        while (not frontier.empty()) {
            const auto current = frontier.front();
            frontier.pop();

            for (auto child : get_node(current).children) {
                flattened.push_back(get_node(child).payload);
                frontier.push(child);
            }
            flattened.push_back(std::nullopt);
        }

        return flattened;
    }

    auto to_string() const -> std::string
    {
        std::stack<std::pair<int, int64_t>> frontier;
        for (auto child : std::views::reverse(storage[0u].children)) {
            frontier.push({0, child});
        }

        std::stringstream ss;

        while (not frontier.empty()) {
            auto [level, current] = frontier.top();
            frontier.pop();

            ss << std::string(static_cast<size_t>(level * 3), ' ')
               << get_node(current).payload << '\n';

            for (auto child : std::views::reverse(get_node(current).children)) {
                frontier.push({level + 1, child});
            }
        }

        return ss.str();
    }

    auto begin() -> iterator { return ++iterator{0, storage}; }

    auto end() -> iterator { return iterator{-1, storage}; }

    auto begin() const -> const_iterator
    {
        return ++const_iterator{0, storage};
    }

    auto end() const -> const_iterator { return const_iterator{-1, storage}; }

    auto cbegin() const -> const_iterator
    {
        return ++const_iterator{0, storage};
    }

    auto cend() const -> const_iterator { return const_iterator{-1, storage}; }

    friend auto operator==(const ImmutableTree& lhs, const ImmutableTree& rhs)
        -> bool
    {
        // Obviously DFS alone cannot be used for comparing trees, so we compare
        // children sizes along with DFS iteration.
        auto left_it = lhs.cbegin();
        auto right_it = rhs.cbegin();

        for (; left_it != lhs.cend() or right_it != rhs.cend();
             ++left_it, ++right_it) {
            if (left_it != lhs.cend() and left_it == rhs.cend()) {
                return false;
            }
            if (left_it == lhs.cend() and right_it != rhs.cend()) {
                return false;
            }
            if (left_it == lhs.cend() and right_it == rhs.cend()) {
                return true;
            }
            if (*left_it != *right_it) {
                return false;
            }
            if (lhs.children(left_it).size() != rhs.children(right_it).size()) {
                return false;
            }
        }

        return true;
    }

private:
    std::vector<Node> storage{Node{}};
    std::queue<int64_t> free_positions;

    auto fix_positions_and_parents(int64_t index, int64_t first)
    {
        auto& children = get_node(index).children;
        if (index == std::ssize(children)) {
            return;
        }
        std::for_each(
            children.begin() + first, children.end(), [&](auto& child) {
                auto& node = get_node(child);
                node.pos = first++;
                node.parent = index;
            });
    }

    auto insert_into_free_spot(Node&& node) -> int64_t
    {
        if (free_positions.empty()) {
            storage.push_back(std::move(node));
            return std::ssize(storage) - 1;
        }
        const auto pos = free_positions.front();
        free_positions.pop();
        get_node(pos) = std::move(node);
        return pos;
    }

    auto mark_removed(int64_t subtree_root) -> void
    {
        std::stack<int64_t> frontier;
        frontier.push(subtree_root);

        while (not frontier.empty()) {
            const auto current = frontier.top();
            frontier.pop();

            free_positions.push(current);

#ifdef __cpp_lib_containers_ranges
            frontier.push_range(get_node(current).children);
#else
            std::ranges::for_each(
                get_node(current).children,
                [&frontier](auto child) { frontier.push(child); });
#endif
        }
    }

    auto find_true_index(const_iterator it) const -> int64_t
    {
        return it == cend() ? 0 : it.ptr;
    }

    auto find_true_index(iterator it) -> int64_t
    {
        return it == end() ? 0 : it.ptr;
    }

    auto get_node(int64_t storage_pos) const -> const Node&
    {
        return storage[static_cast<size_t>(storage_pos)];
    }

    auto get_node(int64_t storage_pos) -> Node&
    {
        return storage[static_cast<size_t>(storage_pos)];
    }
};

static_assert(std::is_copy_constructible_v<ImmutableTree<int>::iterator>);
static_assert(std::is_copy_constructible_v<ImmutableTree<int>::const_iterator>);

// Iterator convertions
static_assert(std::is_convertible_v<ImmutableTree<int>::iterator,
                                    ImmutableTree<int>::iterator>);
static_assert(std::is_convertible_v<ImmutableTree<int>::const_iterator,
                                    ImmutableTree<int>::const_iterator>);
static_assert(std::is_convertible_v<ImmutableTree<int>::iterator,
                                    ImmutableTree<int>::const_iterator>);
static_assert(not std::is_convertible_v<ImmutableTree<int>::const_iterator,
                                        ImmutableTree<int>::iterator>);

// Prevents convertion from iterators from other types
static_assert(not std::is_convertible_v<ImmutableTree<double>::iterator,
                                        ImmutableTree<int>::iterator>);
static_assert(not std::is_convertible_v<ImmutableTree<double>::const_iterator,
                                        ImmutableTree<int>::const_iterator>);

static_assert(
    std::is_trivially_copy_constructible_v<ImmutableTree<int>::iterator>);
static_assert(
    std::is_trivially_copy_constructible_v<ImmutableTree<int>::const_iterator>);

static_assert(std::forward_iterator<ImmutableTree<int>::iterator>);
static_assert(std::forward_iterator<ImmutableTree<int>::const_iterator>);

} // namespace ds

#endif /* end of include guard: IMMUTABLETREE_H_AAE9JBHV */

