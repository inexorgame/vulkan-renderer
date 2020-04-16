# Dependency setup via conan.
# Download conan executer in case it does not exists.
if (NOT EXISTS "${PROJECT_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(
        DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.14/conan.cmake"
        "${PROJECT_BINARY_DIR}/conan.cmake"
    )
endif ()
include(${PROJECT_BINARY_DIR}/conan.cmake)
conan_check(VERSION 1.19.1 REQUIRED)

conan_add_remote(
    NAME bincrafters
    INDEX 1
    URL https://api.bintray.com/conan/bincrafters/public-conan
)

option(INEXOR_CONAN_PROFILE "conan profile" default)
conan_cmake_run(
    CONANFILE conanfile.py
    BASIC_SETUP
    BUILD outdated
    PROFILE ${INEXOR_CONAN_PROFILE}
    PROFILE_AUTO build_type
    KEEP_RPATHS
)
conan_check_compiler()
