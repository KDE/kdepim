set(yousendit_gui_SRCS yousendit_gui.cpp ../../tests/servicetestwidget.cpp ../../tests/testsettingsjob.cpp)
add_executable(yousendit_gui ${yousendit_gui_SRCS})
target_link_libraries(yousendit_gui
  KF5::KIOCore
  KF5::PimCommon
)

