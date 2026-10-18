cmake_minimum_required(VERSION 3.21)
project(uhf_linux VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Core domain depends only on Qt Core (+ Network/Sql later). No GUI here.
find_package(Qt6 REQUIRED COMPONENTS Core)

enable_testing()
# QtTest ships with Qt (already installed) -> zero extra install.
find_package(Qt6 REQUIRED COMPONENTS Test)

# libmpv is optional at configure time: the pure core + its tests build without
# it (lean machines), while the player backend is built when it is present.
find_package(PkgConfig)
set(UHF_HAVE_MPV OFF)
if(PkgConfig_FOUND)
    pkg_check_modules(MPV IMPORTED_TARGET mpv)
    if(MPV_FOUND)
        set(UHF_HAVE_MPV ON)
    endif()
endif()

add_subdirectory(src/core)
if(UHF_HAVE_MPV)
    add_subdirectory(src/mpv)
endif()
add_subdirectory(tests)
