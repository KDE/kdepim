set(webdav_gui_SRCS webdav_gui.cpp ../../tests/servicetestwidget.cpp ../../tests/testsettingsjob.cpp)
add_executable(webdav_gui ${webdav_gui_SRCS})
target_link_libraries(webdav_gui
  KF5::KIOCore
  KF5::PimCommon
)

