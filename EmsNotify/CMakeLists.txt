cmake_minimum_required(VERSION 3.16)

project(EmsNotify VERSION 1.0.2 LANGUAGES CXX)

set(GITHUB_OWNER "harsha-deep")
set(GITHUB_REPO  "EmsNotify")

configure_file(
    include/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    @ONLY
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

qt_standard_project_setup()

qt_add_executable(EmsNotify
    src/main.cpp
    src/tray_manager.cpp
    src/auto_updater.cpp
    include/tray_manager.h
    include/auto_updater.h
    resources/app.qrc
    resources/app.rc
)

target_include_directories(EmsNotify PRIVATE WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(EmsNotify PRIVATE
    Qt6::Widgets
    Qt6::Network
)