find_package(Qt6 REQUIRED COMPONENTS Network)

add_library(uhf_net STATIC
    qt_http_client.cpp
)

target_include_directories(uhf_net PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(uhf_net PUBLIC uhf_core Qt6::Network)
