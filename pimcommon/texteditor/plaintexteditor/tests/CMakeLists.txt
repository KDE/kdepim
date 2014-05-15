include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(plaintexteditor_gui_SRCS plaintexteditor_gui.cpp)
add_executable(plaintexteditor_gui ${plaintexteditor_gui_SRCS})
target_link_libraries(plaintexteditor_gui
  pimcommon
)

