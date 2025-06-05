cmake_minimum_required(VERSION 3.20)
project(XRayDriver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    if(MSVC)
        add_compile_options("/utf-8")
        set(CMAKE_JOB_POOL_LINK link_pool)
        set(CMAKE_JOB_POOLS link_pool=1)
        add_compile_definitions(
            _SILENCE_CXX20_OLD_SHARED_PTR_ATOMIC_SUPPORT_DEPRECATION_WARNING
        )
    endif()
endif()

find_package(Boost 1.83 CONFIG REQUIRED COMPONENTS system filesystem json)
find_package(SQLite3 CONFIG REQUIRED)

find_package(OpenCV REQUIRED COMPONENTS dnn imgproc core)
find_package(spdlog REQUIRED)

include_directories(
    include
    ${Boost_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}   
)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/driver/*.cpp"
    "src/service/*.cpp"
)

set(RESOURCE_FILE "${CMAKE_SOURCE_DIR}/resources/driver.rc")

# ---------------------- Создаём консольный таргет -----------------------
add_executable(XRayDriver
    ${SOURCES}
    ${RESOURCE_FILE}
)
set_target_properties(XRayDriver PROPERTIES
    OUTPUT_NAME         "XRayDriver_console"
    RUNTIME_OUTPUT_NAME "XRayDriver_console"
)

if(WIN32)
    # ----------------- Только для Windows -----------------
    add_executable(XRayDriver_GUI WIN32
        ${SOURCES}
        ${RESOURCE_FILE}
    )
    set_target_properties(XRayDriver_GUI PROPERTIES
        OUTPUT_NAME         "XRayDriver"
        RUNTIME_OUTPUT_NAME "XRayDriver"
    )
    target_compile_definitions(XRayDriver_GUI PRIVATE IS_GUI_APP)
endif()

# ---------------- Объявляем функцию для линковки ----------------
function(link_target TARGET_NAME)
    target_link_libraries(${TARGET_NAME}
        PRIVATE
            Boost::system
            Boost::filesystem
            Boost::json
            SQLite::SQLite3
            spdlog::spdlog
            ${OpenCV_LIBS}    
    )
endfunction()

# Линкуем консольный таргет
link_target(XRayDriver)

# Линкуем GUI-таргет, если он есть
if(WIN32)
    link_target(XRayDriver_GUI)
endif()

# ==================== Installation ====================
set(RESOURCES_DIR ${CMAKE_SOURCE_DIR}/resources)
set(ASSETS_DIR    ${RESOURCES_DIR}/assets)

if (WIN32)
    install(TARGETS
        XRayDriver
        XRayDriver_GUI
        RUNTIME DESTINATION bin
    )
else()
    install(TARGETS
        XRayDriver
        RUNTIME DESTINATION bin
    )
endif()

install(FILES
    ${ASSETS_DIR}/driver.ico
    ${RESOURCES_DIR}/LICENSE.txt
    DESTINATION bin
)

# ==================== CPack/NSIS ====================
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME    "XRAY Driver")
set(CPACK_PACKAGE_VENDOR  "Дедов Дмитрий")
set(CPACK_RESOURCE_FILE_LICENSE "${RESOURCES_DIR}/LICENSE.txt")

if(WIN32)
    set(CPACK_GENERATOR         "NSIS")
    set(CPACK_NSIS_MODIFY_PATH  ON)
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\driver.ico")
    set(CPACK_NSIS_CREATE_ICONS
        "CreateShortCut \\\"$DESKTOP\\\\Драйвер Динамической трубки.lnk\\\" \\\"$INSTDIR\\\\bin\\\\XRayDriver.exe\\\" \\\"\\\" \\\"$INSTDIR\\\\bin\\\\driver.ico\\\""
        "CreateShortCut \\\"$DESKTOP\\\\Драйвер Динамической трубки – Консоль.lnk\\\" \\\"$INSTDIR\\\\bin\\\\XRayDriver_console.exe\\\" \\\"\\\" \\\"$INSTDIR\\\\bin\\\\driver.ico\\\""
    )
endif()

include(CPack)
