set(MODULE test_algorithms)

add_executable(${MODULE})

target_sources(${MODULE}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/alg_ext.test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/optional_ext.test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/string_ext.test.cpp"
)

target_link_libraries(${MODULE}
    PRIVATE
        algorithms
        GTest::gmock_main
    INTERFACE
        project_options
        project_warnings
    )

gtest_discover_tests(${MODULE})
