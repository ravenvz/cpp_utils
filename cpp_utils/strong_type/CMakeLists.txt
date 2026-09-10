set(MODULE strong_type)

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

target_include_directories(${MODULE} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../>
)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

if(ENABLE_TESTING)
endif()
