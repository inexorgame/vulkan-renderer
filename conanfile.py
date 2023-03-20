import os

from conan import ConanFile
from conan.tools.files import copy


class KenophobiaConan(ConanFile):
    name = "Kenophobia"
    version = "0.1.0"

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    options = {
        "build_benchmarks": [True, False],
        "build_tests": [True, False],
        "use_conan_build_tools": [True, False],
    }
    default_options = {
        "build_benchmarks": False,
        "build_tests": False,
        # let conan download all build tools, like CMake, Ninja
        "use_conan_build_tools": False,
    }

    def requirements(self):
        self.requires("glfw/3.3.8")
        self.requires("glm/cci.20220420")
        self.requires("glslang/1.3.236.0")
        self.requires("imgui/1.89.2")
        self.requires("spdlog/1.11.0")
        self.requires("stb/cci.20220909", override=True)
        self.requires("tinygltf/2.5.0")
        self.requires("toml11/3.7.1")
        self.requires("vulkan-headers/1.3.236.0")
        self.requires("vulkan-loader/1.3.236.0")
        self.requires("vulkan-memory-allocator/3.0.1")

        if self.options.build_benchmarks:
            self.requires("benchmark/1.7.1")
        if self.options.build_tests:
            self.requires("gtest/1.13.0")

    def build_requirements(self):
        if self.options.use_conan_build_tools:
            self.tool_requires("cmake/3.25.2")
            self.tool_requires("ninja/1.11.1")

    def validate(self):
        if self.settings.os == "Linux" and self.settings.compiler.libcxx == "libstdc++":
            raise Exception("Inexor is not compatible with libstdc++. "
                            "Please change the 'compiler.libcxx' setting "
                            "(e.g. to libstdc++11).")

    def package(self):
        copy(self, "*.dll", os.path.join(self.build_folder, "bin"), os.path.join(self.package_folder, "bin"))
        copy(self, "*.so*", os.path.join(self.build_folder, "lib"), os.path.join(self.package_folder, "lib"))
