include_directories(${CMAKE_SOURCE_DIR}/libksieve
        ${CMAKE_SOURCE_DIR}/libksieve/ksieveui/scriptsparsing
        )


set(editorsieve_gui_SRCS editorsieve_gui.cpp)
add_executable(editorsieve_gui ${editorsieve_gui_SRCS})
target_link_libraries(editorsieve_gui
  Qt5::Test
  ksieveui
  pimcommon
)

