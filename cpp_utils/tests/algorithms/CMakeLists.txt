set(MODULE_TEST test_algorithms)

add_executable(${MODULE_TEST})

target_sources(${MODULE_TEST}
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/test_find_adjacent.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_fold.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_monadic_optional.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_slide.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_string_utils.cpp"
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
        algorithms
        gmock_main
)

gtest_discover_tests(${MODULE_TEST})

