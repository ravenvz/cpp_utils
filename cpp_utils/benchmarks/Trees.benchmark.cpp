#include <benchmark/benchmark.h>
#include <random>
#include <vector>
#include <optional>
#include "Tree.hpp"
#include "LinearTree.hpp"
// #include <cpp_utils/datastructure/Tree.hpp>
// #include <cpp_utils/datastructure/LinearTree.hpp>

// Generate a randomized, deeply-nested flat representation of a tree for grounding tests
static auto generate_mock_flat_tree(size_t node_count) -> std::vector<std::optional<int>> {
    std::vector<std::optional<int>> mock_data;
    mock_data.push_back(std::nullopt); // Root envelope spacer
    mock_data.push_back(std::nullopt); // Secondary initialization wrapper
    
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> val_dist(1, 100);
    std::uniform_int_distribution<int> branch_dist(1, 5);

    size_t current_count = 0;
    while (current_count < node_count) {
        int children = branch_dist(rng);
        for (int i = 0; i < children && current_count < node_count; ++i) {
            mock_data.push_back(val_dist(rng));
            current_count++;
        }
        mock_data.push_back(std::nullopt); // Close level group
    }
    return mock_data;
}

// ------------------- VECTOR 1: CONSTRUCTION FROM FLATTENED FOOTPRINT -------------------
static void BM_Tree_Construction(benchmark::State& state) {
    auto flat_data = generate_mock_flat_tree(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        auto tree = cpp_utils::datastructure::Tree<int>::from_flattened(flat_data);
        benchmark::DoNotOptimize(tree);
    }
}
BENCHMARK(BM_Tree_Construction)->Range(128, 8192);

static void BM_LinearTree_Construction(benchmark::State& state) {
    auto flat_data = generate_mock_flat_tree(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        auto tree = cpp_utils::datastructure::LinearTree<int>::from_flattened(flat_data);
        benchmark::DoNotOptimize(tree);
    }
}
BENCHMARK(BM_LinearTree_Construction)->Range(128, 8192);

// ------------------- VECTOR 2: DEPTH-FIRST PREORDER TRAVERSAL -------------------
static void BM_Tree_Traversal(benchmark::State& state) {
    auto flat_data = generate_mock_flat_tree(static_cast<size_t>(state.range(0)));
    auto tree = cpp_utils::datastructure::Tree<int>::from_flattened(flat_data);
    
    for (auto _ : state) {
        int64_t sum = 0;
        for (auto val : tree) {
            sum += val;
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Tree_Traversal)->Range(128, 8192);

static void BM_LinearTree_Traversal(benchmark::State& state) {
    auto flat_data = generate_mock_flat_tree(static_cast<size_t>(state.range(0)));
    auto tree = cpp_utils::datastructure::LinearTree<int>::from_flattened(flat_data);
    
    for (auto _ : state) {
        int64_t sum = 0;
        for (auto val : tree) {
            sum += val;
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_LinearTree_Traversal)->Range(128, 8192);

// ------------------- VECTOR 3: SUBTREE DELETION & ERASURE -------------------
static void BM_Tree_Erase(benchmark::State& state) {
    auto flat_data = generate_mock_flat_tree(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        state.PauseTiming();
        auto tree = cpp_utils::datastructure::Tree<int>::from_flattened(flat_data);
        auto it = tree.begin();
        // Advance into an inner child branch to execute a structural delete
        for (int i = 0; i < state.range(0) / 4 && it != tree.end(); ++it, ++i);
        state.ResumeTiming();

        tree.erase(it);
        benchmark::DoNotOptimize(tree);
    }
}
BENCHMARK(BM_Tree_Erase)->Range(128, 8192);

static void BM_LinearTree_Erase(benchmark::State& state) {
    auto flat_data = generate_mock_flat_tree(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        state.PauseTiming();
        auto tree = cpp_utils::datastructure::LinearTree<int>::from_flattened(flat_data);
        auto it = tree.begin();
        for (int i = 0; i < state.range(0) / 4 && it != tree.end(); ++it, ++i);
        state.ResumeTiming();

        tree.erase(it);
        benchmark::DoNotOptimize(tree);
    }
}
BENCHMARK(BM_LinearTree_Erase)->Range(128, 8192);

BENCHMARK_MAIN();
