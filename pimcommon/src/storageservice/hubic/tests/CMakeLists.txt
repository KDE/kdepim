set(hubic_gui_SRCS hubic_gui.cpp ../../tests/servicetestwidget.cpp ../../tests/testsettingsjob.cpp)
add_executable(hubic_gui ${hubic_gui_SRCS})
target_link_libraries(hubic_gui
  KF5::KIOCore
  KF5::PimCommon
)

