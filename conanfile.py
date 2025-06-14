from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
from conan.tools.system.package_manager import Apt, Brew, Chocolatey
import platform

class jetfire27EngineConan(ConanFile):
    name = "jetfire27_engine"
    version = "0.1"
    license = "LGPL"
    settings = "os", "compiler", "build_type", "arch"
    requires = ["boost/1.83.0", "sqlite3/3.45.0", "spdlog/1.14.0", "opencv/4.11.0"]
    generators = "CMakeToolchain", "CMakeDeps"
    exports_sources = (
        "include/*", "src/*", "CMakeLists.txt", "main.cpp",
    )
    if (platform.system() == 'Linux'):
        default_options = {
            "libiconv/*:shared": True,
        }

    def layout(self):
        pass

    def build(self):
        
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
