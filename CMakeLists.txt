cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0167 NEW)
project(XRayDriver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
add_definitions(-D_WIN32_WINNT=0x0601)
    if(MSVC)
        add_compile_options("/utf-8")
    endif()
endif()

find_package(Boost 1.86 CONFIG REQUIRED COMPONENTS system filesystem json)
find_package(SQLite3 CONFIG REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS dnn imgproc core)
find_package(spdlog REQUIRED)

if(APPLE)
    find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
    find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
endif()

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

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Boost::system
        Boost::filesystem
        ${OpenCV_LIBS}
        Boost::json
        SQLite::SQLite3
        spdlog::spdlog
        $<$<PLATFORM_ID:Darwin>:${COCOA_FRAMEWORK}>
        $<$<PLATFORM_ID:Darwin>:${APPKIT_FRAMEWORK}>
)
