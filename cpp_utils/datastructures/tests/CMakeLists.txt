set(MODULE test_datastructures)

add_executable(${MODULE})

target_sources(${MODULE}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/Trees.test.cpp"
        # "${CMAKE_CURRENT_LIST_DIR}/TreeMap.test.cpp"
        # "${CMAKE_CURRENT_LIST_DIR}/UniqueElementsTree.test.cpp"
)
target_link_libraries(${MODULE}
    PRIVATE
        datastructures
        GTest::gmock_main
    INTERFACE
        project_options
        project_warnings
    )

gtest_discover_tests(${MODULE})
