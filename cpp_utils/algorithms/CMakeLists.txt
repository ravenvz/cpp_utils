set(MODULE algorithms)

add_library(${MODULE} INTERFACE)

target_sources(${MODULE}
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include/algorithms/alg_ext.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/algorithms/optional_ext.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/algorithms/ranges_ext.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/algorithms/string_ext.h"
)

target_link_libraries(
    ${MODULE}
    INTERFACE
        project_options
        project_warnings
)

target_include_directories(${MODULE} INTERFACE include)
