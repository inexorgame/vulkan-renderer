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
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/1ed947da9c0207b070c7967af1c60da636039c33/conan.cmake"
        "${CMAKE_BINARY_DIR}/conan.cmake"
        EXPECTED_HASH SHA256=8e1ae613d112105fcb43f2837fe81d6d08b4619237a8035731afebbea9646e32
        TLS_VERIFY ON)
endif()

include(conan)

conan_check(VERSION 1.35.2 REQUIRED)

# TODO: cmake-conan v0.17.0 can automatically set the compiler.cppstd propety based on CMAKE_CXX_STANDARD.
#       We could set this variable globally but it goes against current conventions where the CXX standard is set per target.
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
# if a specific build type is set ignore the multi config generator
if(${is_multi_config} AND NOT DEFINED CMAKE_BUILD_TYPE)
    foreach(TYPE ${CMAKE_CONFIGURATION_TYPES})
        conan_cmake_autodetect(conan_settings BUILD_TYPE ${TYPE})
        conan_cmake_install(
            PATH_OR_REFERENCE ${PROJECT_SOURCE_DIR}
            BUILD outdated
            REMOTE conancenter
            PROFILE ${INEXOR_CONAN_PROFILE}
            GENERATOR cmake_multi
            OPTIONS build_benchmarks=${benchmark_option}
            OPTIONS build_tests=${tests_option}
            SETTINGS ${conan_settings}
            SETTINGS compiler.cppstd=20
            ${compiler_libcxx}
        )
    endforeach()
    include(conanbuildinfo_multi)
else()
    conan_cmake_autodetect(conan_settings)
    conan_cmake_install(
        PATH_OR_REFERENCE ${PROJECT_SOURCE_DIR}
        BUILD outdated
        REMOTE conancenter
        PROFILE ${INEXOR_CONAN_PROFILE}
        OPTIONS build_benchmarks=${benchmark_option}
        OPTIONS build_tests=${tests_option}
        SETTINGS ${conan_settings}
        SETTINGS compiler.cppstd=20
        ${compiler_libcxx}
    )
    include(conanbuildinfo)
endif()

conan_basic_setup()
