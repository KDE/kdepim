include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(box_gui_SRCS box_gui.cpp ../../tests/servicetestwidget.cpp)
add_executable(box_gui ${box_gui_SRCS})
target_link_libraries(box_gui
  Qt5::Test
  KF5::KIOCore
  pimcommon
)

