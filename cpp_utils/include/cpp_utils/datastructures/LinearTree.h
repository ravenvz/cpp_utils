#ifndef IMMUTABLETREE_H_AAE9JBHV
#define IMMUTABLETREE_H_AAE9JBHV

#include "cpp_utils/algorithms/alg_ext.h"
#include "cpp_utils/datastructures/TreeCommon.h"
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

template <std::default_initializable T> class LinearTree {
private:
    struct Node {
        int64_t parent{-1};
        T payload{};
        int64_t pos{0};
        std::vector<int64_t> children{};
    };

public:
    template <typename TransformFunc, typename Proj>
    using TransformResultT = std::remove_cvref_t<
        std::invoke_result_t<TransformFunc,
                             std::invoke_result_t<Proj, const T&>>>;

    template <class v_type, class n_type> class PreorderIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using element_type = v_type;

        friend class PreorderIterator<const v_type, const n_type>;
        friend class LinearTree;

        PreorderIterator() = default;

        PreorderIterator(int64_t p_, std::span<n_type> data_ref)
            : ptr{p_}
            , data{data_ref}
        {
        }

        PreorderIterator(const PreorderIterator&) = default;

        template <class other_n_type, class other_v_type>
            requires(!std::is_const_v<other_n_type> &&
                     std::is_const_v<n_type> &&
                     std::is_same_v<std::remove_const_t<n_type>, other_n_type>)
        PreorderIterator(
            const PreorderIterator<other_v_type, other_n_type>& rhs)
            : ptr{rhs.ptr}
            , prev{rhs.prev}
        {
        }

        auto operator=(const PreorderIterator&) -> PreorderIterator& = default;

        template <class other_n_type, class other_v_type>
            requires(!std::is_const_v<other_n_type> &&
                     std::is_const_v<n_type> &&
                     std::is_same_v<std::remove_const_t<n_type>, other_n_type>)
        auto operator=(const PreorderIterator<other_v_type, other_n_type>& rhs)
            -> PreorderIterator&
        {
            ptr = rhs.ptr;
            prev = rhs.prev;
            data = rhs.data;
            return *this;
        }

        template <class other_n_type, class other_v_type>
            requires(!std::is_const_v<other_n_type> &&
                     std::is_const_v<n_type> &&
                     std::is_same_v<std::remove_const_t<n_type>, other_n_type>)
        PreorderIterator(PreorderIterator<other_v_type, other_n_type>&& rhs)
            : ptr{rhs.ptr}
            , prev{rhs.prev}
            , data{rhs.data}
        {
        }

        template <class other_n_type, class other_v_type>
            requires(!std::is_const_v<other_n_type> &&
                     std::is_const_v<n_type> &&
                     std::is_same_v<std::remove_const_t<n_type>, other_n_type>)
        auto operator=(PreorderIterator<other_v_type, other_n_type>&& rhs)
            -> PreorderIterator&
        {
            ptr = rhs.ptr;
            prev = rhs.prev;
            data = rhs.data;
            return *this;
        }

        auto operator*() const -> element_type&
        {
            return data[static_cast<size_t>(ptr)].payload;
        }

        auto operator->() -> element_type*
        {
            return &data[static_cast<size_t>(ptr)].payload;
        }

        auto operator++() -> PreorderIterator&
        {
            const Node& ptr_node = data[static_cast<size_t>(ptr)];
            const Node& prev_node =
                data[static_cast<size_t>(prev == -1 ? 0 : prev)];
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

        auto operator++(int) -> PreorderIterator
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        friend auto operator==(const PreorderIterator& lhs,
                               const PreorderIterator& rhs) -> bool
        {
            return lhs.ptr == rhs.ptr;
        }

        friend auto operator!=(const PreorderIterator& lhs,
                               const PreorderIterator& rhs) -> bool
        {
            return not(lhs == rhs);
        }

    private:
        int64_t ptr{0};
        int64_t prev{-1};
        std::span<n_type> data;
    };

    using value_type = T;
    using iterator = PreorderIterator<value_type, Node>;
    using const_iterator = PreorderIterator<const value_type, const Node>;

    // LinearTree(std::ranges::input_range auto&& r)
    //     requires(
    //         std::same_as<std::ranges::range_reference_t<T>,
    //         std::optional<T>>)
    //     : LinearTree(std::ranges::begin(r), std::ranges::end(r))
    // {
    // }

    static auto from_flattened(std::ranges::input_range auto&& r) -> LinearTree
    {
        return from_flattened(std::ranges::begin(r), std::ranges::end(r));
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
    static auto from_flattened(I first, S last) -> LinearTree
    // template <std::input_iterator I, std::sentinel_for<I> S>
    //     requires(std::same_as<typename I::value_type, std::optional<T>>)
    // LinearTree(I first, S last)
    {
        std::queue<iterator> frontier;
        LinearTree tree;
        frontier.push(tree.begin());

        for (auto it = first + 2; not frontier.empty() and it != last; ++it) {
            auto parent_it{frontier.front()};
            frontier.pop();
            for (; it->has_value(); ++it) {
                auto child = tree.insert(parent_it, std::forward<T>(**it));
                frontier.push(child);
            }
        }

        return tree;
    }

    LinearTree() = default;

    LinearTree(const LinearTree&) = default;

    auto operator=(const LinearTree&) -> LinearTree& = default;

    LinearTree(LinearTree&&) = default;

    auto operator=(LinearTree&&) -> LinearTree& = default;

    auto insert(iterator parent, T payload) -> iterator
    {
        const auto true_parent = find_true_index(parent);
        const auto pos{std::ssize(get_node(true_parent).children)};
        const auto child_index = insert_into_free_spot(
            Node{true_parent, std::move(payload), pos, std::vector<int64_t>{}});
        get_node(true_parent).children.push_back(child_index);
        return iterator{child_index, storage};
    }

    auto insert(iterator parent, T payload, DestinationPosition insert_pos)
        -> iterator
    {
        const auto true_parent = find_true_index(parent);
        throw_if_invalid_destination(true_parent, insert_pos);
        const auto child_index =
            insert_into_free_spot(Node{true_parent,
                                       std::move(payload),
                                       insert_pos,
                                       std::vector<int64_t>()});
        auto& parent_children = get_node(true_parent).children;
        parent_children.insert(parent_children.begin() + insert_pos,
                               child_index);
        fix_positions_and_parents(true_parent, insert_pos);
        return iterator{child_index, storage};
    }

    auto insert(iterator parent,
                T payload,
                const std::optional<DestinationPosition>& insert_pos)
        -> iterator
    {
        if (insert_pos) {
            return insert(parent, std::move(payload), *insert_pos);
        }
        return insert(parent, std::move(payload));
    }

    template <std::ranges::input_range R, class Proj = std::identity>
    auto insert(iterator parent,
                DestinationPosition insert_pos,
                R&& r,
                Proj proj = {})
    {
        return insert(parent,
                      insert_pos,
                      std::ranges::begin(r),
                      std::ranges::end(r),
                      std::ref(proj));
    }

    template <std::ranges::input_range R, class Proj = std::identity>
    auto insert(iterator parent,
                const std::optional<DestinationPosition>& insert_pos,
                R&& r,
                Proj proj = {})
    {
        return insert(parent,
                      insert_pos,
                      std::ranges::begin(r),
                      std::ranges::end(r),
                      std::ref(proj));
    }

    template <std::input_iterator I,
              std::sentinel_for<I> S,
              class Proj = std::identity>
    auto insert(iterator parent,
                const std::optional<DestinationPosition>& insert_pos,
                I first,
                S last,
                Proj proj = {}) -> iterator
    {
        const auto pos = insert_pos.value_or(DestinationPosition{
            std::ssize(get_node(find_true_index(parent)).children)});
        return insert(parent, pos, first, last, proj);
    }

    template <std::input_iterator I,
              std::sentinel_for<I> S,
              class Proj = std::identity>
    auto insert(iterator parent,
                DestinationPosition insert_pos,
                I first,
                S last,
                Proj proj = {})
    {
        if (first == last) {
            return end();
        }

        const auto true_parent = find_true_index(parent);
        throw_if_invalid_destination(true_parent, insert_pos);

        std::vector<int64_t> indexes(
            static_cast<size_t>(std::distance(first, last)));
        std::transform(
            first, last, indexes.begin(), [&, i = 0](auto&& source) mutable {
                return insert_into_free_spot(Node{true_parent,
                                                  std::invoke(proj, source),
                                                  insert_pos + i++,
                                                  std::vector<int64_t>{}});
            });
        auto& parent_children = get_node(true_parent).children;

#ifdef __cpp_lib_containers_ranges
        parent_children.insert_range(parent_children.begin() + insert_pos,
                                     indexes);
#else
        parent_children.insert(parent_children.begin() + insert_pos,
                               indexes.begin(),
                               indexes.end());
#endif

        fix_positions_and_parents(true_parent, insert_pos);
        return iterator{indexes.front(), storage};
    }

    auto insert_subtree(iterator parent,
                        const LinearTree& other,
                        const std::optional<DestinationPosition>& insert_pos)
        -> void
    {
        insert_subtree(parent,
                       other,
                       insert_pos.value_or(DestinationPosition{std::ssize(
                           get_node(find_true_index(parent)).children)}));
    }

    auto insert_subtree(iterator parent,
                        const LinearTree& other,
                        DestinationPosition insert_pos) -> void
    {
        const auto node_id = find_true_index(parent);
        throw_if_invalid_destination(node_id, insert_pos);

        std::queue<std::pair<iterator, int64_t>> frontier;

        for (auto child_id : other.get_node(0).children) {
            auto it =
                insert(parent, other.get_node(child_id).payload, insert_pos);
            ++insert_pos;
            frontier.push({it, child_id});
        }

        while (not frontier.empty()) {
            auto [parent_it, current] = frontier.front();
            frontier.pop();

            for (auto child_id : other.get_node(current).children) {
                auto it = insert(parent_it, other.get_node(child_id).payload);
                frontier.push({it, child_id});
            }
        }
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

        throw_if_invalid_source(source_parent_index, source_pos, count);
        throw_if_invalid_destination(destination_parent_index, destination_pos);

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

    auto children(iterator it)
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

    auto children_iterators(iterator it)
    {
        const auto index = find_true_index(it);
        return std::views::transform(
            get_node(index).children,
            [this](auto& child_id) { return iterator{child_id, storage}; });
    }

    auto children_iterators(const_iterator it) const
    {
        const auto index = find_true_index(it);
        return std::views::transform(
            get_node(index).children, [this](const auto& child_id) {
                return const_iterator{child_id, storage};
            });
    }

    auto empty() const -> bool { return children(cend()).size() == 0; }

    /* Return number of nodes in the tree in linear time. */
    auto size() const -> int
    {
        return std::ranges::fold_left(
            cbegin(), cend(), 0, [](auto acc, const auto& /* payload */) {
                return acc + 1;
            });
    }

    // Returns subtree with subtree_root as root.
    auto subtree(const_iterator subtree_root) const -> LinearTree
    {
        return transform(subtree_root, std::identity{});
    }

    auto take_subtree(iterator subtree_root) -> LinearTree
    {
        auto tree = transform(subtree_root, std::identity{});
        erase(subtree_root);
        return tree;
    }

    auto position_in_children(const_iterator it) const -> int64_t
    {
        return it == cend() ? 0 : get_node(it.ptr).pos;
    }

    template <typename Func, typename Proj = std::identity>
    auto transform(Func func, Proj proj = {}) const
        -> LinearTree<TransformResultT<Func, Proj>>
    {
        return transform(cend(), func, proj);
    }

    template <typename Func, typename Proj = std::identity>
    auto transform(const_iterator subtree_root, Func func, Proj proj = {}) const
        -> LinearTree<TransformResultT<Func, Proj>>
    {
        using Y = TransformResultT<Func, Proj>;
        LinearTree<Y> mapped;

        std::queue<std::pair<int64_t, typename LinearTree<Y>::iterator>>
            frontier;

        if (subtree_root == cend()) {
            const auto& children = get_node(0).children;

            for (auto child_id : children) {
                auto it = mapped.insert(
                    mapped.end(),
                    std::invoke(func,
                                std::invoke(proj, get_node(child_id).payload)));
                frontier.push({child_id, it});
            }
        }
        else {
            auto it = mapped.insert(
                mapped.end(),
                std::invoke(
                    func,
                    std::invoke(proj, get_node(subtree_root.ptr).payload)));
            frontier.push({subtree_root.ptr, it});
        }

        while (not frontier.empty()) {
            auto [current, mapped_it] = frontier.front();
            frontier.pop();

            const auto& children = get_node(current).children;

            for (auto child_id : children) {
                auto it = mapped.insert(
                    mapped_it,
                    std::invoke(func,
                                std::invoke(proj, get_node(child_id).payload)));
                frontier.push({child_id, it});
            }
        }

        return mapped;
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

    friend auto operator==(const LinearTree& lhs, const LinearTree& rhs) -> bool
    {
        // Obviously traversal alone cannot be used for comparing trees, so we
        // compare children sizes along with iteration.
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

    auto throw_if_invalid_source(int64_t node_id,
                                 SourcePosition source,
                                 Count count) -> void
    {
        if (source < 0 or
            source + count > std::ssize(get_node(node_id).children)) {
            throw std::out_of_range{"Source position out of range"};
        }
    }

    auto throw_if_invalid_destination(int64_t node_id,
                                      DestinationPosition destination) -> void
    {
        if (destination < 0 or
            destination > std::ssize(get_node(node_id).children)) {
            throw std::out_of_range{"Destination out of range"};
        }
    }
};

static_assert(std::is_copy_constructible_v<LinearTree<int>::iterator>);
static_assert(std::is_copy_constructible_v<LinearTree<int>::const_iterator>);

// Iterator convertions
static_assert(std::is_convertible_v<LinearTree<int>::iterator,
                                    LinearTree<int>::iterator>);
static_assert(std::is_convertible_v<LinearTree<int>::const_iterator,
                                    LinearTree<int>::const_iterator>);
static_assert(std::is_convertible_v<LinearTree<int>::iterator,
                                    LinearTree<int>::const_iterator>);
static_assert(not std::is_convertible_v<LinearTree<int>::const_iterator,
                                        LinearTree<int>::iterator>);

// Prevents convertion from iterators from other types
static_assert(not std::is_convertible_v<LinearTree<double>::iterator,
                                        LinearTree<int>::iterator>);
static_assert(not std::is_convertible_v<LinearTree<double>::const_iterator,
                                        LinearTree<int>::const_iterator>);

static_assert(
    std::is_trivially_copy_constructible_v<LinearTree<int>::iterator>);
static_assert(
    std::is_trivially_copy_constructible_v<LinearTree<int>::const_iterator>);

static_assert(std::forward_iterator<LinearTree<int>::iterator>);
static_assert(std::forward_iterator<LinearTree<int>::const_iterator>);

} // namespace ds

#endif /* end of include guard: IMMUTABLETREE_H_AAE9JBHV */

