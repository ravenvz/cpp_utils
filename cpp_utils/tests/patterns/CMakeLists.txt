set(MODULE_TEST test_patterns)

add_executable(${MODULE_TEST})
target_sources(${MODULE_TEST}
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/test_converter.cpp"
)

# This a workaround to prevent linking errors on Windows.
if (WIN32)
    target_compile_definitions( ${MODULE_TEST} PRIVATE -DGTEST_LINKED_AS_SHARED_LIBRARY )
endif()

target_link_libraries(
    ${MODULE_TEST}
    PRIVATE
        project_options
        project_warnings
        gmock_main
    PUBLIC
        cpp_utils
)

gtest_discover_tests(${MODULE_TEST})

