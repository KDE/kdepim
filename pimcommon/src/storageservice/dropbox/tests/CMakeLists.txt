set(dropboxtoken_gui_SRCS dropboxtoken_gui.cpp ../../tests/servicetestwidget.cpp ../../tests/testsettingsjob.cpp )
add_executable(dropboxtoken_gui ${dropboxtoken_gui_SRCS})
target_link_libraries(dropboxtoken_gui
  KF5::KIOCore
  KF5::PimCommon
)

