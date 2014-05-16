include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(webdav_gui_SRCS webdav_gui.cpp ../../tests/servicetestwidget.cpp)
add_executable(webdav_gui ${webdav_gui_SRCS})
target_link_libraries(webdav_gui
  Qt5::Test
  KF5::KIOCore
  pimcommon
)

