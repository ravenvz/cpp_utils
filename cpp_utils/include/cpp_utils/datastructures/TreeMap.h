#ifndef TREE_H_RQOZCKEL
#define TREE_H_RQOZCKEL

#include "cpp_utils/algorithms/alg_ext.h"
#include "cpp_utils/algorithms/optional_ext.h"
#include <concepts>
#include <exception>
#include <functional>
#include <memory>
#include <queue>
#include <ranges>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <utility>
#include <vector>

#include <iostream>

namespace ds {

template <std::default_initializable KeyT, std::default_initializable PayloadT>
class TreeMap {

    template <typename TransformFunc>
    using TransformResultT = std::remove_cvref_t<
        std::invoke_result_t<TransformFunc, const PayloadT&>>;

    struct Node {
        Node(KeyT&& key_, PayloadT&& payload_, Node* parent_) noexcept
            : key{std::move(key_)}
            , payload{std::move(payload_)}
            , parent{parent_}
        {
        }

        Node() = default;

        KeyT key;
        PayloadT payload;
        Node* parent{nullptr};
        std::vector<std::unique_ptr<Node>> children;
    };

    std::unique_ptr<Node> root = std::make_unique<Node>();
    std::unordered_map<KeyT, Node*> registry;

    auto releaseSubTreeMap(std::unique_ptr<Node> n) -> void;

    /* Func is (size_t level, const entry_t& entry) */
    template <typename Func>
    auto for_each(Func func, const Node* initial = nullptr) const -> void;

    auto addChildInternal(std::unique_ptr<Node> child,
                          const std::optional<KeyT>& parent,
                          int64_t insertBeforePosition) -> void;

    auto removeNodesInternal(const std::optional<KeyT>& parent,
                             int64_t row,
                             int64_t count) -> void;

    /* Returns pointer to node with given key if it is in the tree, otherwise
     * thows. Returns pointer to root when key is not given. */
    auto tryLocateNode(const std::optional<KeyT>& key) const -> Node*;

public:
    using entry_t = std::pair<KeyT, PayloadT>;

    TreeMap();

    TreeMap<KeyT, PayloadT> subTreeMap(const KeyT& key) const;

    ~TreeMap() { releaseSubTreeMap(std::move(root)); }

    TreeMap(const TreeMap& other);

    TreeMap& operator=(const TreeMap& other);

    TreeMap(TreeMap&&) noexcept = default;

    TreeMap& operator=(TreeMap&&) noexcept = default;

    auto addChild(KeyT key,
                  PayloadT payload,
                  const std::optional<KeyT>& parent,
                  std::optional<int64_t> insertBeforePosition = std::nullopt)
        -> void;

    auto addSubtree(const TreeMap<KeyT, PayloadT>& addedTreeMap,
                    const std::optional<KeyT>& parent,
                    const std::optional<int64_t>& insertBeforePosition) -> void;

    /* Func is (const PayloadT&) -> TransPayload */
    template <typename Func>
    auto mapped(Func func,
                const std::optional<KeyT>& initial = std::nullopt) const
        -> TreeMap<KeyT, TransformResultT<Func>>;

    /* Return view to all keys in unspecified order. */
    auto keysView() const;

    /* Return view to all payload in unspecified order. */
    auto payloadView() const;

    /* Return view to all entries in unspecified order. */
    auto entriesView() const;

    auto parent(const KeyT& child) const
        -> std::optional<std::reference_wrapper<const KeyT>>;

    auto payload(const KeyT& key) const
        -> std::optional<std::reference_wrapper<const PayloadT>>;

    /* Return view to (keys) children of node with given key. If key is invalid
     * returns view to root's children. */
    auto children(const KeyT& key) const;

    /* Return view to children (keys) of root node (top-level children). */
    auto children() const;

    auto nthChild(const KeyT& key, size_t n) const
        -> std::optional<std::reference_wrapper<const PayloadT>>;

    auto nthChild(size_t n) const
        -> std::optional<std::reference_wrapper<const PayloadT>>;

    // Returns node position among it's parent's children. //
    auto positionInChildren(const KeyT& key) const -> std::optional<size_t>;

    [[nodiscard]] auto leaves() const -> std::vector<PayloadT>;

    /* Func is (const KeyT&, const PayloadT&) -> void */
    template <typename Func>
    auto dfs(Func func, const std::optional<KeyT>& initial = std::nullopt) const
        -> void;

    auto flatten() const -> std::vector<std::optional<entry_t>>;

    static auto unflatten(std::span<const std::optional<entry_t>> flat)
        -> TreeMap;

    auto keys() const { return std::views::keys(registry); }

    auto hasNode(const KeyT& node) const -> bool;

    auto removeNodes(const std::optional<KeyT>& parent,
                     int64_t row,
                     int64_t count) -> void;

    auto removeNode(const KeyT& node) -> void;

    auto moveNodes(const std::optional<KeyT>& sourceParent,
                   int64_t sourceRow,
                   int64_t count,
                   const std::optional<KeyT>& destinationParent,
                   int64_t destinationChild) -> void;

    auto display() const -> std::string;
};

/* --------------------------------------------------
 * --------------------------------------------------
 * --------------------------------------------------*/
// Implementation
/* --------------------------------------------------
 * --------------------------------------------------
 * --------------------------------------------------*/

template <std::default_initializable KeyT, std::default_initializable PayloadT>
TreeMap<KeyT, PayloadT>::TreeMap() = default;

template <std::default_initializable KeyT, std::default_initializable PayloadT>
TreeMap<KeyT, PayloadT>::TreeMap(const TreeMap& other)
{
    *this = other.mapped(std::identity{});
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
TreeMap<KeyT, PayloadT>&
TreeMap<KeyT, PayloadT>::operator=(const TreeMap& other)
{
    *this = other.mapped(std::identity{});
    return *this;
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::addChild(
    KeyT key,
    PayloadT payload,
    const std::optional<KeyT>& parent,
    std::optional<int64_t> insertBeforePosition) -> void
{
    if (hasNode(key)) {
        throw std::runtime_error{"Unique key constraint failed"};
    }
    auto* parentPtr = tryLocateNode(parent);
    auto node =
        std::make_unique<Node>(std::move(key), std::move(payload), parentPtr);
    registry.insert({node->key, node.get()});
    const auto position =
        insertBeforePosition.value_or(parentPtr->children.size());
    parentPtr->children.insert(std::begin(parentPtr->children) + position,
                               std::move(node));
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::addSubtree(
    const TreeMap<KeyT, PayloadT>& addedTreeMap,
    const std::optional<KeyT>& parent,
    const std::optional<int64_t>& insertBeforePosition) -> void
{
    int64_t pos{insertBeforePosition.value_or(0)};

    addedTreeMap.for_each([&](auto level, auto* node) {
        if (level == 0) {
            addChild(node->key, node->payload, parent, pos++);
        }
        else {
            addChild(node->key, node->payload, addedTreeMap.parent(node->key));
        }
    });
}

/* Func is (const PayloadT&) -> TransPayload */
template <std::default_initializable KeyT, std::default_initializable PayloadT>
template <typename Func>
auto TreeMap<KeyT, PayloadT>::mapped(Func func,
                                     const std::optional<KeyT>& initial) const
    -> TreeMap<KeyT, TransformResultT<Func>>
{
    TreeMap<KeyT, TransformResultT<Func>> mappedTreeMap;

    auto transformPayload = [&](auto level, auto* node) {
        // If we are dealing with subTreeMap, parent of the first node would
        // not be found in the mapped tree, so we set it for nullopt for level 0
        auto nodeParent = level == 0 ? std::nullopt : parent(node->key);
        mappedTreeMap.addChild(node->key, func(node->payload), nodeParent);
    };

    for_each(transformPayload, tryLocateNode(initial));

    return mappedTreeMap;
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::keysView() const
{
    return std::views::keys(registry);
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::payloadView() const
{
    return std::views::values(registry) |
           std::views::transform([](auto* node) { return node->payload; });
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::entriesView() const
{
    return std::views::transform(registry, [](const auto& p) {
        return std::make_pair(p.first, p.second->payload);
    });
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::parent(const KeyT& child) const
    -> std::optional<std::reference_wrapper<const KeyT>>
{
    if (auto childIt = registry.find(child); childIt != cend(registry)) {
        if (auto* parentPtr = childIt->second->parent; parentPtr) {
            return parentPtr == root.get()
                       ? std::nullopt
                       : std::optional<std::reference_wrapper<KeyT>>{
                             parentPtr->key};
        }
    }
    throw std::runtime_error{"Asking for parent of non-existing key"};
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::payload(const KeyT& key) const
    -> std::optional<std::reference_wrapper<const PayloadT>>
{
    if (auto it = registry.find(key); it != cend(registry)) {
        return {it->second->payload};
    }
    return std::nullopt;
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::children(const KeyT& key) const
{
    auto it = registry.find(key);
    Node* parent = it != cend(registry) ? it->second : root.get();
    return std::views::transform(parent->children,
                                 [](const auto& ptr) { return ptr->key; });
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::children() const
{
    return std::views::transform(root->children,
                                 [](const auto& ptr) { return ptr->key; });
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::nthChild(const KeyT& key, size_t n) const
    -> std::optional<std::reference_wrapper<const PayloadT>>
{
    if (auto it = registry.find(key); it != cend(registry)) {
        if (n < it->second->children.size()) {
            return it->second->children[n]->payload;
        }
    }
    return std::nullopt;
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::nthChild(size_t n) const
    -> std::optional<std::reference_wrapper<const PayloadT>>
{
    if (n < root->children.size()) {
        return root->children[n]->payload;
    }
    return std::nullopt;
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::positionInChildren(const KeyT& key) const
    -> std::optional<size_t>
{
    auto it = registry.find(key);
    if (it == cend(registry)) {
        return {};
    }

    const Node* parent{it->second->parent == root.get() ? root.get()
                                                        : it->second->parent};
    for (size_t row = 0; const auto& child : parent->children) {
        if (child->key == key) {
            return row;
        }
        ++row;
    }
    return std::nullopt;
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::leaves() const -> std::vector<PayloadT>
{
    std::vector<PayloadT> nodes;
    auto push_leaf = [&nodes](auto /*level*/, const auto* node) {
        if (node->children.empty()) {
            nodes.push_back(node->payload);
        }
    };
    for_each(push_leaf);
    return nodes;
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
template <typename Func>
auto TreeMap<KeyT, PayloadT>::dfs(Func func,
                                  const std::optional<KeyT>& initial) const
    -> void
{
    Node* rt =
        initial.transform([&](const auto& key) { return tryLocateNode(key); })
            .value_or(nullptr);
    for_each([&](int /*level*/,
                 const Node* node) { func(node->key, node->payload); },
             rt);
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::flatten() const
    -> std::vector<std::optional<entry_t>>
{
    std::queue<Node*> frontier;
    frontier.push(root.get());
    std::vector<std::optional<entry_t>> flattened;
    flattened.push_back(std::optional<entry_t>{{root->key, root->payload}});
    flattened.push_back(std::nullopt);

    while (!frontier.empty()) {
        auto* current = frontier.front();
        frontier.pop();
        const auto& children = current->children;
        for (auto& child : children) {
            flattened.push_back(
                std::optional<entry_t>{{child->key, child->payload}});
            frontier.push(child.get());
        }
        flattened.push_back(std::nullopt);
    }

    return flattened;
}

// static //
template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::unflatten(
    std::span<const std::optional<entry_t>> flat) -> TreeMap
{
    TreeMap<KeyT, PayloadT> result;
    std::queue<std::reference_wrapper<const entry_t>> frontier;
    const entry_t fakeroot;
    frontier.push(fakeroot);

    // Starting from 2 as we have empty root node in the tree itself
    // and flattened version also has it.
    for (size_t i{2}; !frontier.empty(); ++i) {
        const auto current = frontier.front();
        frontier.pop();
        for (; flat[i]; ++i) {
            auto& [key, payload] = *flat[i];
            result.addChild(key,
                            payload,
                            &current.get() == &fakeroot ? std::optional<KeyT>{}
                                                        : current.get().first);
            frontier.push(flat[i].value());
        }
    }

    return result;
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::hasNode(const KeyT& node) const -> bool
{
    return registry.contains(node);
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::removeNodes(const std::optional<KeyT>& parent,
                                          int64_t row,
                                          int64_t count) -> void
{
    removeNodesInternal(parent, row, count);
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::removeNode(const KeyT& key) -> void
{
    const auto maybeParent = parent(key);
    if (auto pos = positionInChildren(key); pos) {
        removeNodesInternal(
            maybeParent, static_cast<int64_t>(*positionInChildren(key)), 1);
    }
    else {
        std::string msg{"Error removing node: "};
        msg += key;
        throw std::runtime_error{msg};
    }
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::moveNodes(
    const std::optional<KeyT>& sourceParent,
    int64_t sourceRow,
    int64_t count,
    const std::optional<KeyT>& destinationParent,
    int64_t destinationChild) -> void
{
    if (destinationParent and not hasNode(destinationParent.value())) {
        throw std::runtime_error{"Wrong destination when moving nodes"};
    }

    auto& children = tryLocateNode(sourceParent)->children;

    if (sourceRow + count > static_cast<int64_t>(children.size())) {
        std::string mesg{
            "Indexing error when attempting to move nodes. Parent: "};
        mesg += sourceParent.value_or("null");
        mesg += " row: ";
        mesg += std::to_string(sourceRow);
        mesg += " count: ";
        mesg += std::to_string(count);
        throw std::runtime_error{mesg};
    }

    if (sourceParent == destinationParent) {
        alg::slide(std::begin(children) + sourceRow,
                   std::begin(children) + sourceRow + count,
                   std::begin(children) + destinationChild);
        return;
    }

    auto [first, last] = alg::slide(std::begin(children) + sourceRow,
                                    std::begin(children) + sourceRow + count,
                                    std::end(children));

    std::for_each(first, last, [&](auto& node) {
        addChildInternal(
            std::move(node), destinationParent, destinationChild++);
    });
    children.erase(first, last);
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::display() const -> std::string
{
    std::stringstream ss;
    auto print_node = [&](auto level, auto* current) {
        for (auto i = 0; i < level; ++i) {
            ss << "   ";
        }
        if (current) {
            ss << current->key << " -> " << current->payload << '\n';
        }
    };

    for_each(print_node);

    return ss.str();
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
TreeMap<KeyT, PayloadT>
TreeMap<KeyT, PayloadT>::subTreeMap(const KeyT& key) const
{
    return mapped(std::identity{}, key);
}

template <class CharT, class Traits, class KeyT, class PayloadT>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os,
           const TreeMap<KeyT, PayloadT>& tree)
{
    os << "TreeMap\n" << tree.display();
    return os;
}

/* Return entries in DFS traversal order */
template <typename K, typename P>
auto entriesDFS(const TreeMap<K, P>& tree)
    -> std::vector<typename TreeMap<K, P>::entry_t>
{
    std::vector<typename TreeMap<K, P>::entry_t> result;
    tree.dfs(
        [&result](const auto& key, const auto& payload) {
            result.push_back({key, payload});
        },
        std::nullopt);
    return result;
}

template <typename K, typename P>
auto operator==(const TreeMap<K, P>& lhs, const TreeMap<K, P>& rhs) -> bool
{
    const auto leftEntries = entriesDFS(lhs);
    const auto rightEntries = entriesDFS(rhs);

    return std::ranges::equal(leftEntries, rightEntries);
}

template <typename K,
          typename P,
          typename Comp = std::equal_to<typename TreeMap<K, P>::entry_t>>
auto compare(const TreeMap<K, P>& lhs,
             const TreeMap<K, P>& rhs,
             Comp comp = Comp{}) -> bool
{
    const auto leftEntries = entriesDFS(lhs);
    const auto rightEntries = entriesDFS(rhs);

    return std::ranges::equal(leftEntries, rightEntries, comp);
}

/* --------------------------------------------------
 * --------------------------------------------------
 * --------------------------------------------------
        PRIVATE
 * --------------------------------------------------*/

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::releaseSubTreeMap(std::unique_ptr<Node> n) -> void
{
    if (not n) {
        // Root pointer might not manage memory when TreeMap is in `moved from`
        // state if move was performed.
        return;
    }
    while (n->children.size() > 0) {
        auto leaves = &n->children;
        while (leaves->back()->children.size() > 0) {
            leaves = &leaves->back()->children;
        }
        registry.erase(leaves->back()->key);
        leaves->pop_back();
    }
    registry.erase(n->key);
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
template <typename Func>
auto TreeMap<KeyT, PayloadT>::for_each(Func func, const Node* initial) const
    -> void
{
    std::stack<std::pair<int, const Node*>> frontier;

    // in case when non-root initial root supplied, we add it itself
    if (initial && initial != root.get()) {
        frontier.push({0, initial});
    }
    else { // root node -- we add all it's children, but not root itself
        std::ranges::for_each(std::ranges::reverse_view(root->children),
                              [&](const auto& child) {
                                  frontier.push({0, child.get()});
                              });
    }

    while (!frontier.empty()) {
        auto current = std::move(frontier.top());
        frontier.pop();
        const auto& children = current.second->children;

        func(current.first, current.second);

        for (auto& child : std::ranges::reverse_view(children)) {
            frontier.push({current.first + 1, child.get()});
        }
    }
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::addChildInternal(
    std::unique_ptr<Node> child,
    const std::optional<KeyT>& parent,
    int64_t insertBeforePosition) -> void
{
    auto* parentPtr = tryLocateNode(parent);
    registry.insert({child->key, child.get()});
    child->parent = parentPtr;
    parentPtr->children.insert(
        std::begin(parentPtr->children) +
            std::min(insertBeforePosition,
                     static_cast<int64_t>(parentPtr->children.size())),
        std::move(child));
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::removeNodesInternal(
    const std::optional<KeyT>& parent, int64_t row, int64_t count) -> void
{
    auto* parentPtr = tryLocateNode(parent);
    auto& children = parentPtr->children;
    if (row + count > static_cast<int64_t>(children.size())) {
        std::string mesg{
            "Indexing error when attempting to remove nodes. Parent: "};
        mesg += parent.value_or("null");
        mesg += " row: ";
        mesg += std::to_string(row);
        mesg += " count: ";
        mesg += std::to_string(count);
        throw std::runtime_error{mesg};
    }
    auto [first, last] = alg::slide(std::begin(children) + row,
                                    std::begin(children) + row + count,
                                    std::end(children));
    std::vector<std::unique_ptr<Node>> nodes(static_cast<size_t>(count));
    std::move(first, last, std::begin(nodes));
    children.erase(first, last);
    for (auto& node : nodes) {
        releaseSubTreeMap(std::move(node));
    }
}

template <std::default_initializable KeyT, std::default_initializable PayloadT>
auto TreeMap<KeyT, PayloadT>::tryLocateNode(
    const std::optional<KeyT>& key) const -> Node*
{
    auto nodePtr = [&](const auto& nodeKey) {
        if (auto it = registry.find(nodeKey); it == cend(registry)) {
            std::stringstream ss;
            ss << "Trying to access node that is not in the tree: ";
            ss << nodeKey;
            throw std::runtime_error{ss.str()};
        }
        else {
            return it->second;
        }
    };
    return key.transform(nodePtr).value_or(root.get());
}

} // namespace ds

#endif /* end of include guard: TREE_H_RQOZCKEL */

