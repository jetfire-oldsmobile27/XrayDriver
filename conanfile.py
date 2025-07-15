from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
from conan.tools.system.package_manager import Apt, Brew, Chocolatey
import platform

class jetfire27EngineConan(ConanFile):
    name = "jetfire27_engine"
    version = "0.1"
    license = "LGPL"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    default_options = {
        "libx264/cci.20240224:with_asm": False,
    }

    exports_sources = (
        "include/*", "src/*", "CMakeLists.txt", "main.cpp",
    )
    def requirements(self):
        self.requires("boost/1.83.0")
        self.requires("tgbot/1.8")
        self.requires("sqlite3/3.45.0")
        self.requires("spdlog/1.14.0")
        self.requires("opencv/4.11.0", options={
            "with_v4l": True
        })

    def layout(self):
        pass

    def build(self):
        autotools = Autotools(self)
        autotools.configure_args.append("--disable-doc")
        autotools.configure(args=[
            "--disable-man",
            "--disable-doc",
            "--host=aarch64-linux-gnu",
            "--disable-shared",
            "--enable-static",
            "-x assembler-with-cpp",
            "--prefix=/",
            "--disable-nls",
            "--disable-asm",
            "--disable-bootstrap",
            "HELP2MAN=/bin/true",
            "M4=m4",
            "ac_cv_func_malloc_0_nonnull=yes",
            "ac_cv_func_realloc_0_nonnull=yes",
            "ac_cv_func_reallocarray=no"
        ])
        autotools.make(target="all")
        autotools.install()
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
