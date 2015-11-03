set(gdrive_gui_SRCS gdrive_gui.cpp ../../tests/servicetestwidget.cpp ../../tests/testsettingsjob.cpp)
add_executable(gdrive_gui ${gdrive_gui_SRCS})
target_link_libraries(gdrive_gui
  Qt5::Test
  KF5::KIOCore
  pimcommon
)

