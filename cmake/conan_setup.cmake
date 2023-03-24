# Dependency setup via conan.
if(INEXOR_BUILD_BENCHMARKS)
    set(benchmark_option True)
else()
    set(benchmark_option False)
endif()
if(INEXOR_BUILD_TESTS)
    set(tests_option True)
else()
    set(tests_option False)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
