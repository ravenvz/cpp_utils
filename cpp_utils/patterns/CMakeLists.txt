set(MODULE patterns)

add_library(${MODULE} INTERFACE)

target_sources(${MODULE}
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include/patterns/Converter.h"
)

target_link_libraries(
    ${MODULE}
    INTERFACE
        project_options
        project_warnings
        algorithms
)

target_include_directories(${MODULE} INTERFACE include)
