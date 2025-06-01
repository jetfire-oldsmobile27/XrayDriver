cmake_minimum_required(VERSION 3.16)

project(driver_manager VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Quick)


qt_add_executable(appdriver_manager
    main.cpp
)

qt_add_qml_module(appdriver_manager
    URI driver_manager
    VERSION 1.0
    QML_FILES
        Main.qml
)


set_target_properties(appdriver_manager PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appdriver_manager
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appdriver_manager
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appdriver_manager
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
