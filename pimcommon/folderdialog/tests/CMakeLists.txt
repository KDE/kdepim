include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon
)


set(selectmultiselectiondialog_gui_SRCS selectmultiselectiondialog_gui.cpp)
add_executable(selectmultiselectiondialog_gui ${selectmultiselectiondialog_gui_SRCS})
target_link_libraries(selectmultiselectiondialog_gui
  Qt5::Test
  KF5::Mime
  pimcommon
)

