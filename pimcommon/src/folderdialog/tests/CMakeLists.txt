set(selectmultiselectiondialog_gui_SRCS selectmultiselectiondialog_gui.cpp)
add_executable(selectmultiselectiondialog_gui ${selectmultiselectiondialog_gui_SRCS})
target_link_libraries(selectmultiselectiondialog_gui
  KF5::Mime
  KF5::PimCommon
)
