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
        add_compile_definitions(_SILENCE_CXX20_OLD_SHARED_PTR_ATOMIC_SUPPORT_DEPRECATION_WARNING)
    endif()
endif()

find_package(Boost 1.86 CONFIG REQUIRED COMPONENTS system filesystem json)
find_package(SQLite3 CONFIG REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS dnn imgproc core)
find_package(spdlog REQUIRED)

include_directories(
    include 
    ${Boost_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
)

file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/driver/*.cpp"
    "src/service/*.cpp"
)

# Консольная версия для отладки
add_executable(${PROJECT_NAME} ${SOURCES})

if(WIN32)
    add_executable(${PROJECT_NAME}_GUI WIN32 ${SOURCES})
    set_target_properties(${PROJECT_NAME}_GUI PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
        RUNTIME_OUTPUT_NAME ${PROJECT_NAME}
    )
    target_compile_definitions(${PROJECT_NAME}_GUI PRIVATE IS_GUI_APP)
    
    # Для консольной версии  другое имя
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}_console"
        RUNTIME_OUTPUT_NAME "${PROJECT_NAME}_console"
    )
endif()

function(link_target TARGET)
    target_link_libraries(${TARGET}
        PRIVATE
            Boost::system
            Boost::filesystem
            ${OpenCV_LIBS}
            Boost::json
            SQLite::SQLite3
            spdlog::spdlog
    )
endfunction()

link_target(${PROJECT_NAME})

if(WIN32)
    link_target(${PROJECT_NAME}_GUI)
endif()

# ==================== Installation ====================
set(RESOURCES_DIR ${CMAKE_SOURCE_DIR}/resources)
set(ASSETS_DIR ${RESOURCES_DIR}/assets)

install(TARGETS XRayDriver
    RUNTIME DESTINATION bin
)

install(FILES
    ${ASSETS_DIR}/app.ico
    ${RESOURCES_DIR}/LICENSE.txt  
    DESTINATION bin
)


# ==================== CPack Конфигурация ====================
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME "XRAY Driver")
set(CPACK_PACKAGE_VENDOR "Дедов Дмитрий")
set(CPACK_RESOURCE_FILE_LICENSE "${RESOURCES_DIR}/LICENSE.txt")  
set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\app.ico")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_MODIFY_PATH ON)
    
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        CreateShortCut \\\"$DESKTOP\\\\Драйвер Динамической трубки.lnk\\\" \\
            \\\"$INSTDIR\\\\bin\\\\XRayDriver_console.exe\\\" \\
            \\\"\\\" \\
            \\\"$INSTDIR\\\\bin\\\\app.ico\\\"
    ")
endif()

include(CPack)