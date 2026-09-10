# 1. Declare Google Benchmark dependency via FetchContent safely if not done globally
# Bypasses the git client altogether by extracting an official stable release archive
FetchContent_Declare(
    google_benchmark
    GIT_REPOSITORY "https://github.com/google/benchmark.git"
    GIT_TAG        "v1.9.1"
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE INTERNAL "")
set(BENCHMARK_ENABLE_INSTALL OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(google_benchmark)

# 2. Add the benchmark executable
add_executable(Trees.benchmark Trees.benchmark.cpp)

# 3. Target includes (adjust relative pathing to point back to library header root)
target_include_directories(Trees.benchmark PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../datastructures  # Target where Tree.hpp & ImmutableTree.hpp live
)

# 4. Link Google Benchmark & internal data dependencies
target_link_libraries(Trees.benchmark PRIVATE 
    benchmark::benchmark
    datastructures
)

# 5. Mirror specific release optimization and strict warning profiles for this module
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Trees.benchmark PRIVATE
        -O3
        -Wall 
        -Wextra 
        -Werror
        -Werror=null-dereference
    )
endif()
