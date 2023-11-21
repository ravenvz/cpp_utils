set(MODULE types)

add_library(${MODULE} INTERFACE)

target_sources(${MODULE}
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include/types/NamedType.h"
)

target_link_libraries(
    ${MODULE}
    INTERFACE
        project_options
        project_warnings
        algorithms
)

target_include_directories(${MODULE} INTERFACE include)
