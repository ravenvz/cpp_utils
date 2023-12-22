#ifndef TREE_H_1MKPBXLX
#define TREE_H_1MKPBXLX

#include "cpp_utils/algorithms/alg_ext.h"
#include "cpp_utils/datastructures/TreeCommon.h"
#include "cpp_utils/types/NamedType.h"
#include <algorithm>
#include <concepts>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <queue>
#include <ranges>
#include <span>
#include <sstream>
#include <stack>
#include <utility>
#include <vector>

namespace ds {

template <std::default_initializable T> class Tree {
private:
    struct Node {
        friend class Tree;

        Node() = default;

        Node(T payload_)
            : payload{payload_}
        {
        }

        auto insert(std::unique_ptr<Node> child) -> void
        {
            child->parent = this;
            children.push_back(std::move(child));
            rebuild_position_indexes(std::ssize(children) - 1);
        }

        auto insert(std::unique_ptr<Node> child, DestinationPosition insert_pos)
            -> void
        {
            throw_if_invalid_destination(insert_pos);
            child->parent = this;
            auto insert_it = children.begin() + insert_pos;
            auto first = children.insert(insert_it, std::move(child));
            rebuild_position_indexes(first - children.begin());
        }

        template <typename InputIt>
        auto insert(DestinationPosition insert_pos, InputIt first, InputIt last)
            -> void
        {
            if (first == last) {
                return;
            }
            throw_if_invalid_destination(insert_pos);
            auto insert_it = children.begin() + insert_pos;
            auto it = children.insert(insert_it, first, last);
            rebuild_parent_pointers();
            rebuild_position_indexes(it - children.begin());
        }

        auto take(Node* node) -> std::unique_ptr<Node>
        {
            auto it = std::ranges::find_if(
                children, [&](auto& p) { return p.get() == node; });
            auto subroot = std::move(*it);
            auto first = children.erase(it);
            rebuild_position_indexes(first - children.begin());
            return subroot;
        }

        auto move(SourcePosition source,
                  Count count,
                  DestinationPosition destination) -> void
        {
            throw_if_invalid_source(source, count);
            throw_if_invalid_destination(destination);
            alg::slide(children.begin() + source,
                       children.begin() + source + count,
                       children.begin() + destination);

            rebuild_position_indexes(0);
        }

        auto move(SourcePosition source_pos,
                  Count count,
                  Node* destination,
                  DestinationPosition destination_pos)
        {
            if (destination == this) {
                move(source_pos, count, destination_pos);
                return;
            }

            throw_if_invalid_source(source_pos, count);

            auto [first, last] =
                alg::slide(children.begin() + source_pos,
                           children.begin() + source_pos + count,
                           children.end());
            destination->insert(destination_pos,
                                std::make_move_iterator(first),
                                std::make_move_iterator(last));
            children.erase(first, last);
            rebuild_position_indexes(source_pos);
        }

    private:
        Node* parent{nullptr};
        int64_t pos{0};
        std::vector<std::unique_ptr<Node>> children;
        T payload{};

        auto rebuild_position_indexes(int64_t first)
        {
            std::for_each(
                children.begin() + first,
                children.end(),
                [pos = first](auto& child) mutable { child->pos = pos++; });
        }

        auto rebuild_parent_pointers()
        {
            std::for_each(children.begin(),
                          children.end(),
                          [this](auto& child) { child->parent = this; });
        }

        auto throw_if_invalid_destination(DestinationPosition position) -> void
        {
            if (position < 0 or position > std::ssize(children)) {
                throw std::out_of_range{"Destination out of range"};
            }
        }

        auto throw_if_invalid_source(SourcePosition source_position,
                                     Count count)
        {
            if (source_position < 0 or
                source_position + count > std::ssize(children)) {
                throw std::out_of_range{"Source position out of range"};
            }
        }

        auto fit_destination(DestinationPosition position) -> int64_t
        {
            return std::clamp(static_cast<int64_t>(position),
                              int64_t{0},
                              std::ssize(children));
        }
    };

public:
    template <typename TransformFunc>
    using TransformResultT =
        std::remove_cvref_t<std::invoke_result_t<TransformFunc, const T&>>;

    template <typename v_type, typename node_type> class DfsIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using element_type = v_type;

        friend class DfsIterator<const v_type, const node_type>;
        friend class Tree;

        DfsIterator() = default;

        explicit DfsIterator(node_type* p_)
            : ptr{p_}
        {
        }

        DfsIterator(const DfsIterator&) = default;

        // Conversion constructor that permits convertion from iterator to
        // const_iterator but is disabled to prevent convertion from
        // const_iterator to iterator
        template <typename n_type, typename nv_type>
            requires(!std::is_const_v<n_type> && std::is_const_v<node_type> &&
                     std::is_same_v<std::remove_const_t<node_type>, n_type>)
        DfsIterator(const DfsIterator<nv_type, n_type>& rhs)
            : ptr{rhs.ptr}
            , prev{rhs.prev}
        {
        }

        auto operator*() const -> element_type& { return ptr->payload; }

        auto operator->() -> element_type* { return &ptr->payload; }

        auto operator++() -> DfsIterator&
        {
            auto* tmp = ptr;
            ptr = bottom_reached() ? ptr->parent : next_node();
            prev = tmp;

            // Node has been visited before, skip it
            if (ptr and prev != ptr->parent) {
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
            return not(lhs.ptr == rhs.ptr);
        }

    private:
        node_type* ptr{nullptr};
        node_type* prev{nullptr};
        typename std::vector<std::unique_ptr<Node>>::iterator p_it;

        auto bottom_reached() const -> bool
        {
            // ptr points to leaf without siblings to the right
            return ptr->children.empty() or
                   (prev != ptr->parent and
                    (prev->pos + 1 >= std::ssize(ptr->children)));
        }

        auto next_node() const -> node_type*
        {
            // Pick first child if came from parent or next child if came from
            // child.
            const auto offset =
                static_cast<size_t>(prev == ptr->parent ? 0 : prev->pos + 1);
            return ptr->children[offset].get();
        }
    };

    using value_type = T;
    using iterator = DfsIterator<value_type, Node>;
    using const_iterator = DfsIterator<const value_type, const Node>;

    Tree() = default;

    Tree(std::ranges::input_range auto&& r)
        : Tree{std::ranges::begin(r), std::ranges::end(r)}
    {
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
    Tree(I first, S last)
    {
        std::queue<iterator> frontier;
        frontier.push(begin());

        ++first;
        ++first;

        for (; not frontier.empty() and first != last; ++first) {
            auto parent_it = frontier.front();
            frontier.pop();
            for (; first->has_value(); ++first) {
                auto it = insert(parent_it, first->value());
                frontier.push(it);
            }
        }
    }

    ~Tree() { release_subtree(std::move(root)); }

    Tree(const Tree& other)
    {
        if (this == &other) {
            return;
        }
        *this = other.transform(std::identity{});
    }

    Tree(Tree&& other) = default;

    auto operator=(Tree&& other) -> Tree& = default;

    auto operator=(const Tree& other) -> Tree&
    {
        if (this == &other) {
            return *this;
        }
        root = std::move(other.transform(std::identity{}).root);
        return *this;
    }

    auto insert(iterator parent, T payload) -> iterator
    {
        auto* true_parent{parent == end() ? root.get() : parent.ptr};
        auto child = std::make_unique<Node>(std::move(payload));
        auto* child_ptr = child.get();
        true_parent->insert(std::move(child));
        return iterator{child_ptr};
    }

    auto insert(iterator parent, T payload, DestinationPosition insert_pos)
        -> iterator
    {
        auto* true_parent{parent == end() ? root.get() : parent.ptr};
        auto child = std::make_unique<Node>(std::move(payload));
        auto* child_ptr = child.get();
        true_parent->insert(std::move(child), insert_pos);
        return iterator{child_ptr};
    }

    auto insert(iterator parent,
                T payload,
                const std::optional<DestinationPosition>& insert_pos)
        -> iterator
    {
        return insert_pos ? insert(parent, std::move(payload), *insert_pos)
                          : insert(parent, std::move(payload));
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
        const auto pos = insert_pos.value_or(
            DestinationPosition{std::ssize(parent.ptr->children)});
        return insert(parent, pos, first, last, proj);
    }

    template <std::input_iterator I,
              std::sentinel_for<I> S,
              class Proj = std::identity>
    auto insert(iterator parent,
                DestinationPosition insert_pos,
                I first,
                S last,
                Proj proj = {}) -> iterator
    {
        if (first == last) {
            return end();
        }
        auto* true_parent{parent == end() ? root.get() : parent.ptr};
        std::vector<std::unique_ptr<Node>> buffer(
            static_cast<size_t>(last - first));
        std::transform(first, last, buffer.begin(), [&](auto&& source) {
            return std::make_unique<Node>(std::invoke(proj, source));
        });
        auto* ptr = buffer.front().get();
        true_parent->insert(insert_pos,
                            std::make_move_iterator(buffer.begin()),
                            std::make_move_iterator(buffer.end()));
        return iterator{ptr};
    }

    auto insert_subtree(iterator parent,
                        const Tree& other,
                        const std::optional<DestinationPosition>& insert_pos)
        -> void
    {
        insert_subtree(parent,
                       other,
                       insert_pos.value_or(DestinationPosition{
                           std::ssize(parent.ptr->children)}));
    }

    auto insert_subtree(iterator parent,
                        const Tree& other,
                        DestinationPosition insert_pos) -> void
    {
        std::queue<std::pair<iterator, const Node*>> frontier;

        for (const auto& child : other.root->children) {
            auto child_it = insert(parent, child->payload, insert_pos);
            ++insert_pos;
            frontier.push({child_it, child.get()});
        }

        while (not frontier.empty()) {
            auto [it, other_ptr] = frontier.front();
            frontier.pop();

            for (const auto& child : other_ptr->children) {
                auto child_it = insert(it, child->payload);
                frontier.push({child_it, child.get()});
            }
        }
    }

    auto erase(iterator subtree_root) -> void
    {
        if (subtree_root == end()) {
            return;
        }
        release_subtree(take_subtree(subtree_root).root);
    }

    auto move_nodes(iterator source_parent,
                    SourcePosition source_pos,
                    Count count,
                    iterator destination_parent,
                    DestinationPosition destination_pos) -> void
    {
        auto* source_parent_ptr{source_parent == end() ? root.get()
                                                       : source_parent.ptr};
        auto* destination_parent_ptr{
            destination_parent == end() ? root.get() : destination_parent.ptr};

        source_parent_ptr->move(
            source_pos, count, destination_parent_ptr, destination_pos);
    }

    auto parent(const_iterator it) const -> const_iterator
    {
        if (it == cend() or it.ptr->parent == root.get()) {
            return cend();
        }
        return const_iterator{it.ptr->parent};
    }

    auto parent(iterator it) -> iterator
    {
        if (it == end() or it.ptr->parent == root.get()) {
            return end();
        }
        return iterator{it.ptr->parent};
    }

    auto children(iterator it)
    {
        auto* true_ptr = it == end() ? root.get() : it.ptr;
        return std::views::transform(true_ptr->children,
                                     [](auto& node) { return node->payload; });
    }

    auto children(const_iterator it) const
    {
        auto* true_ptr = it == cend() ? root.get() : it.ptr;
        return std::views::transform(
            true_ptr->children, [](const auto& node) { return node->payload; });
    }

    auto take_subtree(iterator subtree_root) -> Tree
    {
        auto* parent = subtree_root.ptr->parent;
        Tree subtree;
        subtree.root->insert(parent->take(subtree_root.ptr));
        return subtree;
    }

    auto position_in_children(const_iterator it) const -> int64_t
    {
        return it == end() ? 0 : it.ptr->pos;
    }

    template <typename Func>
    auto transform(Func func) const -> Tree<TransformResultT<Func>>
    {
        return transform(cend(), func);
    }

    template <typename Func>
    auto transform(const_iterator subtree_root, Func func) const
        -> Tree<TransformResultT<Func>>
    {
        using Y = TransformResultT<Func>;
        Tree<Y> mapped;

        std::queue<std::pair<const Node*, typename Tree<Y>::iterator>> frontier;
        if (subtree_root == cend()) {
            for (auto& child : root->children) {
                auto mapped_it =
                    mapped.insert(mapped.end(), func(child->payload));
                frontier.push({child.get(), mapped_it});
            }
        }
        else {
            auto mapped_it =
                mapped.insert(mapped.end(), func(subtree_root.ptr->payload));
            frontier.push({subtree_root.ptr, mapped_it});
        }

        while (not frontier.empty()) {
            auto [current, mapped_it] = frontier.front();
            frontier.pop();

            for (auto& child : current->children) {
                auto it = mapped.insert(mapped_it, func(child->payload));
                frontier.push({child.get(), it});
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

        std::stack<Node*> frontier;
        frontier.push(subtree_root.ptr);

        while (not frontier.empty()) {
            auto* current = frontier.top();
            frontier.pop();

            func(current->payload);
            std::ranges::for_each(current->children, [&frontier](auto& child) {
                frontier.push(child.get());
            });
        }
    }

    auto flatten() const -> std::vector<std::optional<T>>
    {
        std::queue<const Node*> frontier;
        frontier.push(root.get());

        std::vector<std::optional<T>> flattened{std::nullopt, std::nullopt};

        while (not frontier.empty()) {
            const auto* current = frontier.front();
            frontier.pop();

            for (const auto& child : current->children) {
                flattened.push_back(child->payload);
                frontier.push(child.get());
            }
            flattened.push_back(std::nullopt);
        }

        return flattened;
    }

    auto to_string() const -> std::string
    {
        std::stack<std::pair<int, Node*>> frontier;
        for (auto& child : std::views::reverse(root->children)) {
            frontier.push({0, child.get()});
        }

        std::stringstream ss;

        while (not frontier.empty()) {
            auto [level, current] = frontier.top();
            frontier.pop();

            if (current) {
                ss << std::string(static_cast<size_t>(level * 3), ' ')
                   << current->payload << '\n';
            }

            for (auto& child : std::views::reverse(current->children)) {
                frontier.push({level + 1, child.get()});
            }
        }

        return ss.str();
    }

    auto begin() -> iterator { return ++iterator(root.get()); }

    auto end() -> iterator { return iterator(nullptr); }

    auto begin() const -> const_iterator
    {
        return ++const_iterator(root.get());
    }

    auto end() const -> const_iterator { return const_iterator(nullptr); }

    auto cbegin() const -> const_iterator
    {
        return ++const_iterator(root.get());
    }

    auto cend() const -> const_iterator { return const_iterator(nullptr); }

    friend auto operator==(const Tree& lhs, const Tree& rhs) -> bool
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
    std::unique_ptr<Node> root{std::make_unique<Node>()};

    auto release_subtree(std::unique_ptr<Node> subtree_root) -> void
    {
        if (not subtree_root) {
            return;
        }

        while (not subtree_root->children.empty()) {
            auto leaves = &subtree_root->children;
            while (not leaves->back()->children.empty()) {
                leaves = &leaves->back()->children;
            }
            leaves->pop_back();
        }
    }
};

static_assert(std::is_copy_constructible_v<Tree<int>::iterator>);
static_assert(std::is_copy_constructible_v<Tree<int>::const_iterator>);

// Iterator convertions
static_assert(std::is_convertible_v<Tree<int>::iterator, Tree<int>::iterator>);
static_assert(std::is_convertible_v<Tree<int>::const_iterator,
                                    Tree<int>::const_iterator>);
static_assert(
    std::is_convertible_v<Tree<int>::iterator, Tree<int>::const_iterator>);
static_assert(
    not std::is_convertible_v<Tree<int>::const_iterator, Tree<int>::iterator>);

// Prevents convertion from iterators from other types
static_assert(
    not std::is_convertible_v<Tree<double>::iterator, Tree<int>::iterator>);
static_assert(not std::is_convertible_v<Tree<double>::const_iterator,
                                        Tree<int>::const_iterator>);

static_assert(std::is_trivially_copy_constructible_v<Tree<int>::iterator>);
static_assert(
    std::is_trivially_copy_constructible_v<Tree<int>::const_iterator>);

static_assert(std::forward_iterator<Tree<int>::iterator>);
static_assert(std::forward_iterator<Tree<int>::const_iterator>);

} // namespace ds

#endif /* end of include guard: TREE_H_1MKPBXLX */
