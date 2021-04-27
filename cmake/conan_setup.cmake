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
if(UNIX)
    set(compiler_libcxx SETTINGS compiler.libcxx=libstdc++11)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0140fdbe363abfc1993f579e8d711067ac4742e1/conan.cmake"
        "${CMAKE_BINARY_DIR}/conan.cmake"
        EXPECTED_HASH SHA256=DDF0FAFACF48B5C4912ECCE5701C252532437C40277734CAD5F4A8084470ADBC
        TLS_VERIFY ON)
endif()

include(conan)

conan_check(VERSION 1.35.2 REQUIRED)

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
# if a specific build type is set ignore the multi config generator
if(${is_multi_config} AND NOT DEFINED CMAKE_BUILD_TYPE)
    foreach(TYPE ${CMAKE_CONFIGURATION_TYPES})
        conan_cmake_autodetect(conan_settings BUILD_TYPE ${TYPE})
        conan_cmake_install(
            PATH_OR_REFERENCE ${PROJECT_SOURCE_DIR}
            BUILD outdated
            REMOTE conan-center
            PROFILE ${INEXOR_CONAN_PROFILE}
            GENERATOR cmake_multi
            OPTIONS build_benchmarks=${benchmark_option}
            OPTIONS build_tests=${tests_option}
            SETTINGS ${conan_settings}
            SETTINGS compiler.cppstd=17
            ${compiler_libcxx}
        )
    endforeach()
    include(conanbuildinfo_multi)
else()
    conan_cmake_autodetect(conan_settings)
    conan_cmake_install(
        PATH_OR_REFERENCE ${PROJECT_SOURCE_DIR}
        BUILD outdated
        REMOTE conan-center
        PROFILE ${INEXOR_CONAN_PROFILE}
        OPTIONS build_benchmarks=${benchmark_option}
        OPTIONS build_tests=${tests_option}
        SETTINGS ${conan_settings}
        SETTINGS compiler.cppstd=17
        ${compiler_libcxx}
    )
    include(conanbuildinfo)
endif()

conan_basic_setup()
