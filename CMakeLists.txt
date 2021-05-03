cmake_minimum_required(VERSION 3.5)
#set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
project(klm LANGUAGES CXX C)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
option(BUILD_SHARED_LIBS "" NO)
add_subdirectory(quazip)
add_compile_definitions(
        TAGLIB_STATIC
)
include_directories(
        taglib/taglib
        taglib/taglib/toolkit
        taglib/taglib/mpeg/id3v2
        taglib/taglib/mpeg/id3v2/frames
        taglib/taglib/mpeg/id3v1
        taglib/taglib/mod
        taglib/taglib/ogg
        taglib/taglib/ogg/flac
        taglib/taglib/ogg/opus
        taglib/taglib/ogg/speex
        taglib/taglib/ogg/vorbis
        taglib/taglib/flac
        taglib/taglib/ape
        taglib/taglib/riff
        taglib/taglib/asf
        taglib/taglib/mpeg
        taglib/taglib/riff
        taglib/taglib/riff/aiff
        taglib/taglib/riff/wav
        taglib/taglib/it
        taglib/taglib/mp4
        taglib/taglib/mpc
        taglib/taglib/s3m
        taglib/taglib/trueaudio
        taglib/taglib/wavpack
        taglib/taglib/xm
)
option(WITH_ZLIB "Build with ZLIB" OFF)
option(ENABLE_STATIC_RUNTIME "" ON)
add_subdirectory(taglib)

add_subdirectory(spdlog)

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
