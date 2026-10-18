add_library(uhf_core STATIC
    sources/m3u_parser.cpp
    sources/xtream_api.cpp
    player/player_controller.cpp
    library/library_index.cpp
)

target_include_directories(uhf_core PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(uhf_core PUBLIC Qt6::Core)
