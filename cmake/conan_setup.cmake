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
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
        "${CMAKE_BINARY_DIR}/conan.cmake"
        EXPECTED_HASH SHA256=396e16d0f5eabdc6a14afddbcfff62a54a7ee75c6da23f32f7a31bc85db23484
        TLS_VERIFY ON)
endif()

include(conan)

conan_check(VERSION 1.35.2 REQUIRED)
conan_cmake_autodetect(conan_settings BUILD_TYPE ${CMAKE_BUILD_TYPE})
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
conan_basic_setup()
