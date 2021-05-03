cmake_minimum_required(VERSION 3.5)
#set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
project(klm LANGUAGES CXX C)

option(BUILD_SHARED_LIBS "" NO)
add_subdirectory(quazip)

option(WITH_ZLIB "Build with ZLIB" OFF)
option(ENABLE_STATIC_RUNTIME "" ON)
add_subdirectory(taglib)

add_subdirectory(spdlog)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Sql REQUIRED)

add_executable(klm
  main.cpp
  mainwindow.cpp
  mainwindow.h
  mainwindow.ui
  karaokefile.cpp
  karaokefile.h
  dupefindercrc.cpp
  dupefindercrc.h
)

target_link_libraries(klm PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql tag spdlog::spdlog_header_only QuaZip::QuaZip)
