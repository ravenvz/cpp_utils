set(MODULE libdatastructure)

add_library(${MODULE} INTERFACE)
add_library(${PROJECT}::${MODULE} ALIAS ${MODULE})
target_link_libraries(${MODULE}
    INTERFACE
        project_options
        project_warnings
)

target_sources(${MODULE}
INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/LinearTree.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/Tree.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/TreeCommon.hpp"
    # "${CMAKE_CURRENT_LIST_DIR}/TreeMap.hpp"
    # "${CMAKE_CURRENT_LIST_DIR}/UniqueElementsTree.hpp"
)

target_include_directories(${MODULE} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../..")

if(ENABLE_TESTING)
    find_package(GTest REQUIRED)

    add_executable(test_${MODULE})

    target_sources(test_${MODULE}
        PRIVATE
            # "${CMAKE_CURRENT_LIST_DIR}/Tree.test.cpp"
            # "${CMAKE_CURRENT_LIST_DIR}/TreeMap.test.cpp"
            # "${CMAKE_CURRENT_LIST_DIR}/UniqueElementsTree.test.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/Trees.test.cpp"
    )

    target_link_libraries(test_${MODULE}
        PRIVATE
            ${MODULE}
            GTest::gmock_main
            # libstrongtype
        )
    
    gtest_discover_tests(test_${MODULE})
endif()
