#ifndef UNIQUEELEMENTSTREE_H_UZNKGFBF
#define UNIQUEELEMENTSTREE_H_UZNKGFBF

#include <algorithm>
#include <format>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <queue>
#include <ranges>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <iostream>

namespace ds {

struct UniqueKeyError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct KeyError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

/*
 *
 *
 *
 */
template <std::default_initializable PayloadT,
          std::default_initializable Selector = std::identity>
class UniqueElementsTree {
private:
    using internal_id_t = int32_t;

    struct Node {
        Node* parent{nullptr};
        PayloadT payload{};
        std::vector<Node*> children;
    };

public:
    using key_t =
        std::remove_cvref_t<std::invoke_result_t<Selector, const PayloadT&>>;
    using maybe_key = std::optional<key_t>;

    template <typename Func>
    using TransformResultT =
        std::remove_cvref_t<std::invoke_result_t<Func, const PayloadT&>>;

    using const_dfs_iterator_type = const PayloadT;
    using dfs_iterator_type = PayloadT;

    template <typename const_tag> struct DfsIterator {
        using iterator_category = std::forward_iterator_tag;
        using element_type = const_tag;
        using difference_type = std::ptrdiff_t;

        DfsIterator()
            : ptr{nullptr}
        {
        }

        DfsIterator(Node* p_)
            : ptr{p_}
        {
            if (ptr != nullptr) {
                ++(*this);
            }
        }

        auto operator*() const -> element_type& { return ptr->payload; }

        auto operator++() -> DfsIterator&
        {
            for (auto* child : std::ranges::reverse_view(ptr->children)) {
                frontier.push(child);
            }
            if (not frontier.empty()) {
                ptr = frontier.top();
                frontier.pop();
            }
            else {
                ptr = nullptr;
            }
            return *this;
        }

        auto operator++(int) -> DfsIterator
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        // operator DfsIterator<const_dfs_iterator_type>() const
        // {
        //     return DfsIterator<const_dfs_iterator_type>{ptr};
        // }

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
        Node* ptr;
        std::stack<Node*> frontier;
    };

    auto cbegin_dfs() const -> DfsIterator<const_dfs_iterator_type>
    {
        return DfsIterator<const_dfs_iterator_type>{root.get()};
    }

    auto cend_dfs() const -> DfsIterator<const_dfs_iterator_type>
    {
        return DfsIterator<const_dfs_iterator_type>{nullptr};
    }

    auto begin_dfs() const -> DfsIterator<const_dfs_iterator_type>
    {
        return DfsIterator<const_dfs_iterator_type>{root.get()};
    }

    auto end_dfs() const -> DfsIterator<const_dfs_iterator_type>
    {
        return DfsIterator<const_dfs_iterator_type>{nullptr};
    }

    auto begin_dfs() -> DfsIterator<dfs_iterator_type>
    {
        return DfsIterator<dfs_iterator_type>{root.get()};
    }

    auto end_dfs() -> DfsIterator<dfs_iterator_type>
    {
        return DfsIterator<dfs_iterator_type>{nullptr};
    }

    friend auto operator==(const UniqueElementsTree& lhs,
                           const UniqueElementsTree& rhs) -> bool
    {
        // TODO replace with iterator compare
        return lhs.flatten() == rhs.flatten();
    }

    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os, const Node& node)
    {
        os << "Node{parent: " << node.parent << " payload: " << node.payload;
        return os;
    }

    auto add_child(PayloadT payload,
                   const maybe_key& parent = std::nullopt,
                   const std::optional<size_t>& pos = std::nullopt) -> void
    {
        const auto key = selector(payload);

        // Key already exists
        if (has_key(key)) {
            throw UniqueKeyError{std::format(
                "Unique key constraint failed: {} ", selector(payload))};
        }

        if (parent and not has_key(parent.value())) {
            throw KeyError{std::format("No parent with key: {}", *parent)};
        }

        auto* parent_ptr = parent
                               .transform([this](const auto& parent_key) {
                                   return registry[parent_key].get();
                               })
                               .value_or(root.get());

        const auto insert_pos =
            static_cast<int>(pos.value_or(parent_ptr->children.size()));

        auto node = std::make_unique<Node>(parent_ptr, std::move(payload));
        parent_ptr->children.insert(
            std::begin(parent_ptr->children) + insert_pos, node.get());
        registry.insert({key, std::move(node)});
    }

    // Flattens the three into a vector interleaving with null nodes in order to
    // differentiate children of different nodes so that the tree could be then
    // restored.
    //
    // Mainly useful for serialization.
    auto flatten() const -> std::vector<std::optional<PayloadT>>
    {
        std::queue<const Node*> frontier;
        frontier.push(root.get());
        std::vector<std::optional<PayloadT>> flattened;
        flattened.push_back(std::nullopt);
        flattened.push_back(std::nullopt);

        while (not frontier.empty()) {
            const auto* current = frontier.front();
            frontier.pop();
            for (auto* child : current->children) {
                flattened.push_back(child->payload);
                frontier.push(child);
            }
            flattened.push_back(std::nullopt);
        }

        return flattened;
    }

    static auto unflatten(std::span<const std::optional<PayloadT>> flat)
        -> UniqueElementsTree
    {
        UniqueElementsTree<PayloadT, Selector> result;
        std::queue<const PayloadT*> frontier;
        const PayloadT fakeroot{};
        frontier.push(&fakeroot);
        Selector selector;

        for (size_t i{2}; not frontier.empty(); ++i) {
            const auto* current = frontier.front();
            frontier.pop();
            for (; flat[i]; ++i) {
                const auto& payload = *flat[i];
                result.add_child(payload,
                                 current == &fakeroot ? maybe_key{}
                                                      : selector(*current));
                frontier.push(&flat[i].value());
            }
        }

        return result;
    }

    // template <typename Func, typename NewSelector>
    // auto map(Func func) -> UniqueElementsTree<TransformResultT<Func>,
    // NewSelector>
    // {
    //     UniqueElementsTree<TransformResultT<Func>, NewSelector> mappedTree;
    //
    //     auto transformPayload = [&](auto level, auto* node) {
    //         mappedTree.addChildInternal(func(node->payload),
    //                                     mappedTree.parent(node));
    //     };
    //
    //     return mappedTree;
    // }
    //

    /* Apply UnaryFunction for side-effects to each element of a subtree with
     * root at initial node.
     *
     * Function signature is:
     *
     * auto f(const PayloadT&) -> void
     *
     */
    // template <typename UnaryFunc>
    // auto dfs(UnaryFunc&& func, const maybe_key& initial = std::nullopt) const
    //     -> void
    // {
    //         const auto* start = initial
    //                                 .transform([this](const auto& key) {
    //                                     if (not nodes.contains(key)) {
    //                                         throw KeyError{std::format(
    //                                             "No such key exists: {}",
    //                                             key)};
    //                                     }
    //                                     return &nodes[key];
    //                                 })
    //                                 .value_or(&root);
    //         for_each([&](int #<{(| dfs_level |)}>#,
    //                      const Node* node) { func(node->payload); },
    //                  *start);
    // }

    auto has_key(const key_t& key) const -> bool
    {
        return registry.contains(key);
    }

    auto to_string() const -> std::string
    {
        std::stringstream ss;
        auto print_node = [&](auto level, const auto& current) {
            for (auto i = 0; i < level; ++i) {
                ss << "   ";
            }
            if (current) {
                ss << *current << '\n';
                // ss << current->payload << '\n';
            }
        };

        for_each_dfs(print_node, *root);

        return ss.str();
    }

private:
    std::unique_ptr<Node> root = std::make_unique<Node>();
    Selector selector;
    std::unordered_map<key_t, std::unique_ptr<Node>> registry;

    /* Apply Func for side-effects to each element of a subtree with root at
     * inital node.
     *
     * Function signature is:
     *
     * auto f(int, const Node*) -> void), where int is a level of dfs
     * traversal
     */
    template <typename Func>
    auto for_each_dfs(Func func, const Node& initial) const -> void
    {
        std::stack<std::pair<int, const Node*>> frontier;

        if (&initial != root.get()) {
            frontier.push({0, &initial});
        }
        else {
            std::ranges::for_each(std::ranges::reverse_view(root->children),
                                  [&](auto* child) {
                                      frontier.push({0, child});
                                  });
        }

        while (not frontier.empty()) {
            const auto& current = frontier.top();
            frontier.pop();

            func(current.first, current.second);

            const auto& children = current.second->children;

            for (auto* child : std::ranges::reverse_view(children)) {
                frontier.push({current.first + 1, child});
            }
        }
    }
};

template <class C> auto cbegin_dfs(const C& c) -> decltype(c.cbegin_dfs())
{
    return c.cbegin_dfs();
}

template <class C> auto begin_dfs(const C& c) -> decltype(c.begin_dfs())
{
    return c.begin_dfs();
}

template <class C> auto begin_dfs(C& c) -> decltype(c.begin_dfs())
{
    return c.begin_dfs();
}

template <class C> auto cend_dfs(C& c) -> decltype(c.cend_dfs())
{
    return c.cend_dfs();
}

template <class C> auto end_dfs(C& c) -> decltype(c.end_dfs())
{
    return c.end_dfs();
}

template <class C> auto end_dfs(const C& c) -> decltype(c.end_dfs())
{
    return c.end_dfs();
}
// template <typename PayloadT, typename SelectorT>
// auto ordered_entries(const UniqueElementsTree<PayloadT, SelectorT>& tree)
//     -> std::vector<PayloadT>
// {
//     std::vector<PayloadT> result;
//     tree.dfs([&](const auto& pld) { result.push_back(pld); });
//     return result;
// }

// template <typename PayloadT, typename SelectorT>
// inline auto begin_dfs(const UniqueElementsTree<PayloadT, SelectorT>& tree)
//     -> UniqueElementsTree<PayloadT, SelectorT>::DfsIterator<PayloadT>
// {
//     return tree.begin_dfs();
// }
//
// template <typename PayloadT, typename SelectorT>
// inline auto cbegin_dfs(const UniqueElementsTree<PayloadT, SelectorT>& tree)
//     -> UniqueElementsTree<PayloadT, SelectorT>::DfsIterator<const PayloadT>
// {
//     return tree.cbegin_dfs();
// }
//
// template <typename PayloadT, typename SelectorT>
// inline auto end_dfs(const UniqueElementsTree<PayloadT, SelectorT>& tree)
//     -> UniqueElementsTree<PayloadT, SelectorT>::DfsIterator<PayloadT>
// {
//     return tree.end_dfs();
// }
//
// template <typename PayloadT, typename SelectorT>
// inline auto cend_dfs(const UniqueElementsTree<PayloadT, SelectorT>& tree)
//     -> UniqueElementsTree<PayloadT, SelectorT>::DfsIterator<const PayloadT>
// {
//     return tree.cend_dfs();
// }

} // namespace ds

#endif /* end of include guard: UNIQUEELEMENTSTREE_H_UZNKGFBF */

