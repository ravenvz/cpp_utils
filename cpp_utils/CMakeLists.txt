add_subdirectory(algorithms)
add_subdirectory(strong_type)
add_subdirectory(patterns)
add_subdirectory(datastructures)

if (ENABLE_BENCHMARKS) 
    add_subdirectory(benchmarks)
endif()
