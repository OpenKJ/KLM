cmake_minimum_required(VERSION 3.5)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
project(klm LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(PkgConfig)

find_package(miniz CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

pkg_check_modules(TAGLIB REQUIRED IMPORTED_TARGET taglib)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Sql REQUIRED)

add_executable(klm
  main.cpp
  mainwindow.cpp
  mainwindow.h
  mainwindow.ui
  karaokefile.cpp
  karaokefile.h
  karaokefileinfo.cpp
  karaokefileinfo.h
  mzarchive.cpp
  mzarchive.h
  okarchive.cpp
  okarchive.h
  tagreader.cpp
  tagreader.h
  dupefindercrc.cpp
  dupefindercrc.h
)


target_link_libraries(klm PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql miniz::miniz PkgConfig::TAGLIB spdlog::spdlog spdlog::spdlog_header_only)
