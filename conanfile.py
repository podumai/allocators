from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout

class Containers(ConanFile):
    settings: tuple[str] = ("os", "compiler", "build_type", "arch")
    generators: tuple[str] = ("CMakeDeps",)

    def requirements(self) -> None:
        self.requires("catch2/3.11.0")

    def generate(self) -> None:
        tc = CMakeToolchain(self)
        tc.generator = "Ninja"
        tc.variables["CMAKE_CXX_STANDARD"] = "23"
        tc.generate()

    def build(self) -> None:
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def layout(self) -> None:
        cmake_layout(self)
