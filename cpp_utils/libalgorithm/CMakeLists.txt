set(MODULE libalgorithm)

add_library(${MODULE} INTERFACE)
add_library(${PROJECT}::${MODULE} ALIAS ${MODULE})
target_link_libraries(${MODULE}
    INTERFACE
        project_options
        project_warnings
)

target_sources(${MODULE}
INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/alg_ext.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/optional_ext.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/string_ext.hpp"
)

target_include_directories(${MODULE} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../..")

if(ENABLE_TESTING)
    add_executable(test_${MODULE})

    target_sources(test_${MODULE}
        PRIVATE
            "${CMAKE_CURRENT_LIST_DIR}/alg_ext.test.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/optional_ext.test.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/string_ext.test.cpp"
    )

    target_link_libraries(test_${MODULE}
        PRIVATE
            ${MODULE}
            GTest::gmock_main
        INTERFACE
            project_options
            project_warnings
            # libstrongtype
        )
    
    gtest_discover_tests(test_${MODULE})
endif()
