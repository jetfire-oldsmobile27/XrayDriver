cmake_minimum_required(VERSION 3.12)
project(XRayDriver LANGUAGES CXX)

find_package(Boost 1.85 REQUIRED COMPONENTS system asio log)
find_package(nlohmann_json 3.11 REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/commands.cpp                    
    src/eventnotifier.cpp               
    src/xrayprotocolstrategy.cpp
    src/commandservice.cpp              
    src/httpserver.cpp                  
    src/singleton.cpp                   
    src/xraytubecontroller.cpp
    src/daemonizer.cpp                  
    src/iprotocolstrategy.cpp           
    src/syncverifier.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Boost::boost
    Boost::asio
    Boost::system
    Boost::log
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)