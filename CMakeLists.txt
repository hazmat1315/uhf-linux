cmake_minimum_required(VERSION 3.21)
project(uhf_linux VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Work around Qt6's FindWrapOpenGL hard-coding "-framework AGL", which newer
# macOS SDKs have removed. Qt's module early-returns if this target already
# exists, so we pre-create it (GLOBAL) linking only the OpenGL framework.
if(APPLE)
    find_package(OpenGL QUIET)
    if(NOT TARGET WrapOpenGL::WrapOpenGL)
        add_library(WrapOpenGL::WrapOpenGL INTERFACE IMPORTED GLOBAL)
        if(TARGET OpenGL::GL)
            target_link_libraries(WrapOpenGL::WrapOpenGL INTERFACE OpenGL::GL)
        else()
            target_link_libraries(WrapOpenGL::WrapOpenGL INTERFACE "-framework OpenGL")
        endif()
        set(WrapOpenGL_FOUND ON)
    endif()
endif()

# Core domain depends on Qt Core; networking adds Network; the view-models run
# fetches off the UI thread with Concurrent.
find_package(Qt6 REQUIRED COMPONENTS Core Network Concurrent)

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
add_subdirectory(src/net)
if(UHF_HAVE_MPV)
    add_subdirectory(src/mpv)
endif()
add_subdirectory(src/app)
add_subdirectory(tests)
