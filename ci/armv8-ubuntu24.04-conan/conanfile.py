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
        autotools = Autotools(self)
        autotools.configure(args=[
            "--host=aarch64-linux-gnu",
            "--disable-shared",
            "--enable-static",
            "--prefix=/",
            "--disable-nls",
            "--disable-bootstrap",
            "HELP2MAN=/bin/true",
            "M4=m4",
            "ac_cv_func_malloc_0_nonnull=yes",
            "ac_cv_func_realloc_0_nonnull=yes",
            "ac_cv_func_reallocarray=no"
        ])
        autotools.make()
        autotools.install()
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
