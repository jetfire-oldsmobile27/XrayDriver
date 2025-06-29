from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
from conan.tools.system.package_manager import Apt, Brew, Chocolatey
import platform

class jetfire27EngineConan(ConanFile):
    name = "jetfire27_engine"
    version = "0.1"
    license = "LGPL"
    settings = "os", "compiler", "build_type", "arch"
    requires = ["boost/1.83.0", "sqlite3/3.45.0", "spdlog/1.14.0", "opencv/4.9.0"]
    generators = "CMakeToolchain", "CMakeDeps"
    default_options = {
    "*:shared": False,
    "*:fPIC": True,
    "libx264/*:asm": False,
    # Оставляем только нужные модули
    "opencv/*:with_eigen": False,
    "opencv/*:with_ffmpeg": False,
    "opencv/*:with_gstreamer": False,
    "opencv/*:with_gtk": False,
    "opencv/*:with_gtk2": False,
    "opencv/*:with_qt": False,
    "opencv/*:with_cuda": False,
    "opencv/*:with_opencl": False,
    "opencv/*:with_openmp": False,
    "opencv/*:with_tbb": False,
    "opencv/*:with_ipp": False,
    "opencv/*:with_webp": False,
    "opencv/*:with_jpeg": False,
    "opencv/*:with_png": False,
    "opencv/*:with_tiff": False,
    "opencv/*:with_openexr": False,
    "opencv/*:with_java": False,
    "opencv/*:with_python": False,
    "opencv/*:with_python2": False,
    "opencv/*:with_python3": False,
    }

    exports_sources = (
        "include/*", "src/*", "CMakeLists.txt", "main.cpp",
    )

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
            "--disable-asm",
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
