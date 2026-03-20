from conan import ConanFile
import os
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout

class Tema1(ConanFile):
    settings = ("os", "compiler", "build_type", "arch")
    generators = ("CMakeToolchain", "CMakeDeps")

    name = "Tema_3"
    version = "1.0"
    author = "Balcus Bogdan"
    exports_sources = "src/*"

    def requirements(self):
        self.requires("libconfig/1.7.3")
    
    def layout(self):
        cmake_layout(self)
    
    def generate(self):
        pass
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
    
    def package(self):
        cmake = CMake(self)
        cmake.install()
    
    def package_info(self):
        self.cpp_info.libs = ["tema_3"]
