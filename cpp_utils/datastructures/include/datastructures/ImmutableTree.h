#ifndef IMMUTABLETREE_H_AAE9JBHV
#define IMMUTABLETREE_H_AAE9JBHV

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

        template <class other_n_type, class other_v_type, class w>
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
            // const auto parent = data[static_cast<size_t>(ptr)].parent;
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
    using const_iterator = DfsIterator<const T, const Node>;

    template <typename ForwardIt> ImmutableTree(ForwardIt first, ForwardIt last)
    {
        std::queue<int64_t> frontier;
        frontier.push(0);

        for (auto it = first + 2; not frontier.empty() and first != last;
             ++it) {
            const auto parent{frontier.front()};
            frontier.pop();
            for (; it->has_value(); ++it) {
                const auto child = insert(parent, std::move(it->value()));
                frontier.push(child);
            }
        }
    }

    ImmutableTree() = default;

    ImmutableTree(const ImmutableTree&) = default;

    auto operator=(const ImmutableTree&) -> ImmutableTree& = default;

    ImmutableTree(ImmutableTree&&) = default;

    auto operator=(ImmutableTree&&) -> ImmutableTree& = default;

    auto insert(int64_t parent, T payload) -> int64_t
    {
        const auto child_index{std::ssize(storage)};
        const auto pos{
            std::ssize(storage[static_cast<size_t>(parent)].children)};
        storage.emplace_back(
            parent, std::move(payload), pos, std::vector<int64_t>{});
        auto& parent_children = storage[static_cast<size_t>(parent)].children;
        parent_children.insert(parent_children.begin() + pos, child_index);
        return child_index;
    }

    auto parent(const_iterator it) const -> const_iterator
    {
        if (storage[static_cast<size_t>(it.ptr)].parent == 0 or it == cend()) {
            return cend();
        }
        return const_iterator{storage[static_cast<size_t>(it.ptr)].parent,
                              storage};
    }

    auto children(const_iterator it) const
    {
        size_t index = it == cend() ? 0u : static_cast<size_t>(it.ptr);
        return std::views::transform(
            storage[index].children, [this](const auto& node) {
                return storage[static_cast<size_t>(node)].payload;
            });
    }

    template <typename Func>
    auto transform(Func func) const -> ImmutableTree<TransformResultT<Func>>
    {
        ImmutableTree<TransformResultT<Func>> mapped;
        std::queue<int64_t> frontier;
        frontier.push(0);

        while (not frontier.empty()) {
            const auto current = frontier.front();
            frontier.pop();

            for (auto child : storage[static_cast<size_t>(current)].children) {
                mapped.insert(
                    current, func(storage[static_cast<size_t>(child)].payload));
                frontier.push(child);
            }
        }

        return mapped;
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
               << storage[static_cast<size_t>(current)].payload << '\n';

            for (auto child : std::views::reverse(
                     storage[static_cast<size_t>(current)].children)) {
                frontier.push({level + 1, child});
            }
        }

        return ss.str();
    }

    auto position_in_children(const_iterator it) const -> int64_t
    {
        return it == cend() ? 0 : storage[static_cast<size_t>(it.ptr)].pos;
    }

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

private:
    std::vector<Node> storage{Node{}};
};

static_assert(std::is_copy_constructible_v<ImmutableTree<int>::const_iterator>);
static_assert(
    std::is_trivially_copy_constructible_v<ImmutableTree<int>::const_iterator>);
static_assert(std::forward_iterator<ImmutableTree<int>::const_iterator>);

} // namespace ds

#endif /* end of include guard: IMMUTABLETREE_H_AAE9JBHV */

