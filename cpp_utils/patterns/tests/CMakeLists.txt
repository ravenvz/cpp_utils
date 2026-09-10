set(MODULE test_patterns)

add_executable(${MODULE})

target_sources(${MODULE}
        PRIVATE
            "${CMAKE_CURRENT_LIST_DIR}/CommonState.hpp"
            "${CMAKE_CURRENT_LIST_DIR}/Converter.test.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/Mediator.test.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/Observer.test.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/State.ExternalTransitions.test.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/State.InternalTransitions.test.cpp"
)

target_link_libraries(${MODULE}
    PRIVATE
        patterns
        GTest::gmock_main
    INTERFACE
        project_options
        project_warnings
    )

gtest_discover_tests(${MODULE})
