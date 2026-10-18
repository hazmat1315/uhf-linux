# View-model library (Qt Core only) — shared by the GUI app and the tests, so
# the file->model->play wiring is verifiable headlessly.
add_library(uhf_viewmodels STATIC
    viewmodels/app_controller.cpp
    viewmodels/channel_list_model.cpp
)
target_include_directories(uhf_viewmodels PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(uhf_viewmodels PUBLIC uhf_core Qt6::Core)

# The GUI executable is only built when libmpv (the player backend) is present.
if(UHF_HAVE_MPV)
    find_package(Qt6 REQUIRED COMPONENTS Gui Qml Quick QuickControls2)
    set(CMAKE_AUTORCC ON)

    add_executable(uhf_app
        main.cpp
        resources.qrc
    )
    target_include_directories(uhf_app PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(uhf_app PRIVATE
        uhf_viewmodels
        uhf_mpv
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
    )
endif()
