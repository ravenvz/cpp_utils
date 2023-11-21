add_library(cpp_utils INTERFACE)

target_include_directories(cpp_utils
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_subdirectory(algorithms)
add_subdirectory(datastructures)
add_subdirectory(patterns)
add_subdirectory(types)

target_link_libraries(
    cpp_utils
    INTERFACE
        date_wrapper_options
        date_wrapper_warnings
        algorithms
        datastructures
        patterns
        types
)
