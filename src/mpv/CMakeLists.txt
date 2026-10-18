add_library(uhf_mpv STATIC
    mpv_backend.cpp
)

target_include_directories(uhf_mpv PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(uhf_mpv PUBLIC uhf_core PkgConfig::MPV)
