set(PROJECT_TESTS cpp_utils_tests)
project(${PROJECT_TESTS})

# add_subdirectory(types)
# add_subdirectory(algorithms)
# add_subdirectory(datastructures)
# add_subdirectory(patterns)

find_package(Threads REQUIRED)

# Download and unpack googletest at configure time
include(cmake/GtestDownloader.cmake)

target_include_directories(gmock_main SYSTEM BEFORE INTERFACE
    "$<BUILD_INTERFACE:${gtest_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${gmock_SOURCE_DIR}/include>")
