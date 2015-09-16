include_directories(${libksieve_SOURCE_DIR}
        ${libksieve_SOURCE_DIR}/ksieveui/scriptsparsing
        )


set(editorsieve_gui_SRCS editorsieve_gui.cpp)
add_executable(editorsieve_gui ${editorsieve_gui_SRCS})
target_link_libraries(editorsieve_gui
  Qt5::Test
  ksieveui
  KF5::PimCommon
)

