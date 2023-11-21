add_library(cpp_utils INTERFACE)

target_include_directories(cpp_utils
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_sources(cpp_utils
INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/include/algorithms/alg_ext.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/algorithms/optional_ext.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/algorithms/ranges_ext.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/algorithms/string_ext.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/datastructures/ImmutableTree.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/datastructures/Tree.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/patterns/Converter.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/types/NamedType.h"
)

target_link_libraries(cpp_utils
    INTERFACE
        project_options
        project_warnings
)
