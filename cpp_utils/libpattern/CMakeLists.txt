set(MODULE libpattern)

add_library(${MODULE} INTERFACE)
add_library(${PROJECT}::${MODULE} ALIAS ${MODULE})
target_link_libraries(${MODULE}
    INTERFACE
        project_options
        project_warnings
)

target_sources(${MODULE}
INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/Converter.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Mediator.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Observer.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/State.hpp"
)

target_include_directories(${MODULE} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../..")

if(ENABLE_TESTING)
    add_executable(test_${MODULE})

    target_sources(test_${MODULE}
        PRIVATE
            "${CMAKE_CURRENT_LIST_DIR}/CommonState.test.hpp"
            "${CMAKE_CURRENT_LIST_DIR}/Converter.test.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/Mediator.test.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/Observer.test.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/State.ExternalTransitions.test.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/State.InternalTransitions.test.cpp"
    )

    target_link_libraries(test_${MODULE}
        PRIVATE
            ${MODULE}
            GTest::gmock_main
        INTERFACE
            project_options
            project_warnings
        )
    
    gtest_discover_tests(test_${MODULE})
endif()
