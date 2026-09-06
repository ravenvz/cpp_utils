set(MODULE libstrongtype)

add_library(${MODULE} INTERFACE)
add_library(${PROJECT}::${MODULE} ALIAS ${MODULE})
target_link_libraries(${MODULE}
    INTERFACE
        project_options
        project_warnings
)

target_sources(${MODULE}
INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/StrongType.hpp"
)

target_include_directories(${MODULE} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../..")

if(ENABLE_TESTING)
    add_executable(test_${MODULE} StrongType.test.cpp)

    target_sources(test_${MODULE}
        PRIVATE
            "${CMAKE_CURRENT_LIST_DIR}/StrongType.test.cpp"
    )
    target_link_libraries(test_${MODULE}
        PRIVATE
            ${MODULE}
            GTest::gmock_main
        )
    
    gtest_discover_tests(test_${MODULE})
endif()
