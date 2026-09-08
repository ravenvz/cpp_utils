add_subdirectory(libalgorithm)
add_subdirectory(libstrongtype)
add_subdirectory(libpattern)
add_subdirectory(libdatastructure)

if (ENABLE_BENCHMARKS) 
    add_subdirectory(benchmarks)
endif()
