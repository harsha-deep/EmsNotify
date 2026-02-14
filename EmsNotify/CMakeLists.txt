cmake_minimum_required(VERSION 3.16)

project(EmsNotify LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

qt_standard_project_setup()

qt_add_executable(EmsNotify
    src/main.cpp
    src/tray_manager.cpp
    include/tray_manager.h
    resources/app.qrc
)

target_include_directories(EmsNotify PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(EmsNotify PRIVATE
    Qt6::Widgets
    Qt6::Network
)
