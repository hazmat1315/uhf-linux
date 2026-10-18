# Helper: one test executable per test file, registered with CTest.
function(uhf_add_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE uhf_core Qt6::Test)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

uhf_add_test(test_m3u_parser)
uhf_add_test(test_xtream_api)
uhf_add_test(test_player_controller)
uhf_add_test(test_library_index)

# View-model wiring (Qt Core only; no libmpv needed).
add_executable(test_app_controller test_app_controller.cpp)
target_link_libraries(test_app_controller PRIVATE uhf_viewmodels Qt6::Test)
add_test(NAME test_app_controller COMMAND test_app_controller)

# Player backend smoke test only when libmpv is available.
if(UHF_HAVE_MPV)
    add_executable(test_mpv_backend test_mpv_backend.cpp)
    target_link_libraries(test_mpv_backend PRIVATE uhf_mpv Qt6::Test)
    add_test(NAME test_mpv_backend COMMAND test_mpv_backend)
endif()
