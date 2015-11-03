set(box_gui_SRCS box_gui.cpp ../../tests/servicetestwidget.cpp ../../tests/testsettingsjob.cpp)
add_executable(box_gui ${box_gui_SRCS})
target_link_libraries(box_gui
  KF5::KIOCore
  KF5::PimCommon
)

