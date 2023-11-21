#ifndef TREE_H_1MKPBXLX
#define TREE_H_1MKPBXLX

#include "algorithms/alg_ext.h"
#include "types/NamedType.h"
#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <queue>
#include <ranges>
#include <span>
#include <sstream>
#include <stack>
#include <utility>
#include <vector>

#include <limits>

namespace ds {

struct CountTag { };
using Count = types::ImplicitNamedType<int64_t, CountTag>;

struct SourcePosTag { };
using SourcePosition = types::ImplicitNamedType<int64_t, SourcePosTag>;

struct DestinationPosTag { };
using DestinationPosition =
    types::ImplicitNamedType<int64_t, DestinationPosTag>;

template <std::default_initializable PayloadT> class Tree {
private:
    struct Node {
        friend class Tree;

        Node() = default;

        Node(PayloadT payload_)
            : payload{payload_}
        {
        }

        auto insert(std::unique_ptr<Node> child,
                    const std::optional<DestinationPosition>& position) -> void
        {
            child->parent = this;
            const auto num_children{static_cast<int64_t>(children.size())};
            const auto destination_pos{static_cast<int64_t>(
                position.value_or(DestinationPosition{num_children}))};
            const auto insert_pos{std::min(num_children, destination_pos)};
            auto insert_it = children.begin() + insert_pos;
            auto first = children.insert(insert_it, std::move(child));
            rebuild_position_indexes(first - children.begin());
        }

        template <typename InputIt>
        auto insert(DestinationPosition position, InputIt first, InputIt last)
            -> void
        {
            if (first == last) {
                return;
            }
            const auto insert_pos = std::min(
                DestinationPosition{static_cast<int64_t>(children.size())},
                position);
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
        PayloadT payload{};

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
    };

public:
    template <typename TransformFunc>
    using TransformResultT = std::remove_cvref_t<
        std::invoke_result_t<TransformFunc, const PayloadT&>>;

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

        auto operator->() -> element_type* { return &ptr; }

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

    using value_type = PayloadT;
    using iterator = DfsIterator<value_type, Node>;
    using const_iterator = DfsIterator<const value_type, const Node>;

    Tree() = default;

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

    auto insert(iterator parent,
                PayloadT payload,
                const std::optional<DestinationPosition>& pos = std::nullopt)
        -> iterator
    {
        auto* true_parent{parent == end() ? root.get() : parent.ptr};
        auto child = std::make_unique<Node>(std::move(payload));
        auto* child_ptr = child.get();
        true_parent->insert(std::move(child), pos);
        return iterator{child_ptr};
    }

    auto take_subtree(iterator subtree_root) -> Tree
    {
        auto* parent = subtree_root.ptr->parent;
        Tree subtree;
        subtree.root->insert(parent->take(subtree_root.ptr), std::nullopt);
        return subtree;
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

    auto erase(iterator subtree_root) -> void
    {
        release_subtree(take_subtree(subtree_root).root);
    }

    auto position_in_children(const_iterator it) const -> int64_t
    {
        return it == end() ? 0 : it.ptr->pos;
    }

    template <typename Func>
    auto transform(Func func) const -> Tree<TransformResultT<Func>>
    {
        using Y = TransformResultT<Func>;
        Tree<Y> mapped;
        std::queue<std::pair<const Node*, typename Tree<Y>::iterator>> frontier;
        frontier.push({root.get(), mapped.end()});

        while (not frontier.empty()) {
            auto entry = frontier.front();
            auto* current = entry.first;
            auto mapped_it = entry.second;
            frontier.pop();

            for (const auto& child : current->children) {
                auto it = mapped.insert(mapped_it, func(child->payload));
                frontier.push({child.get(), it});
            }
        }

        return mapped;
    }

    friend auto operator==(const Tree& lhs, const Tree& rhs) -> bool
    {
        return std::ranges::equal(lhs, rhs);
    }

    auto flatten() const -> std::vector<std::optional<PayloadT>>
    {
        std::queue<const Node*> frontier;
        frontier.push(root.get());

        std::vector<std::optional<PayloadT>> flattened{std::nullopt,
                                                       std::nullopt};

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

    static auto unflatten(std::span<const std::optional<PayloadT>> flat) -> Tree
    {
        Tree<PayloadT> restored;
        std::queue<iterator> frontier;
        frontier.push(restored.begin());

        for (size_t i{2}; not frontier.empty(); ++i) {
            auto parent_it = frontier.front();
            frontier.pop();
            for (; flat[i]; ++i) {
                auto it =
                    restored.insert(parent_it, std::move(flat[i].value()));
                frontier.push(it);
            }
        }

        return restored;
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
