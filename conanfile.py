from conans import ConanFile, CMake

class InexorConan(ConanFile):

    settings = (
        "os",
        "compiler",
        "build_type",
        "arch"
    )

    requires = (
        "benchmark/1.5.0",
        "glm/0.9.9.7",
        "gtest/1.10.0",
        "spdlog/1.5.0",
        "glfw/3.3.2@bincrafters/stable",
        "toml11/3.1.0",
        "imgui/1.75",
        "assimp/5.0.1",
        "enet/1.3.14",
        "stb/20190512@conan/stable",
        "zlib/1.2.11",
        "nlohmann_json/3.7.3",
    )

    generators = "cmake"

    def imports(self):
        # Copies all dll files from packages bin folder to my "bin" folder (win)
        self.copy("*.dll", dst="bin", src="bin")
        # Copies all dylib files from packages lib folder to my "lib" folder (macosx)
        self.copy("*.dylib*", dst="lib", src="lib") # From lib to lib
        # Copies all so files from packages lib folder to my "lib" folder (linux)
        self.copy("*.so*", dst="lib", src="lib") # From lib to lib

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
