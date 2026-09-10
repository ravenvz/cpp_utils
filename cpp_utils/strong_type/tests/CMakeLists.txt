set(MODULE test_strong_type)

add_executable(${MODULE})

target_sources(${MODULE}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/StrongType.test.cpp"
)

target_link_libraries(${MODULE}
    PRIVATE
        strong_type
        GTest::gmock_main
    INTERFACE
        project_options
        project_warnings
    )

gtest_discover_tests(${MODULE})
