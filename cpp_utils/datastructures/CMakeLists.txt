set(MODULE datastructures)

add_library(${MODULE} INTERFACE)

target_sources(${MODULE}
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include/datastructures/ImmutableTree.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/datastructures/Tree.h"
)

target_link_libraries(
    ${MODULE}
    INTERFACE
        project_options
        project_warnings
        algorithms
        types
)

target_include_directories(${MODULE} INTERFACE include)
