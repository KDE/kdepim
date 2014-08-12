include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(dropboxtoken_gui_SRCS dropboxtoken_gui.cpp ../../tests/servicetestwidget.cpp )
add_executable(dropboxtoken_gui ${dropboxtoken_gui_SRCS})
target_link_libraries(dropboxtoken_gui
  Qt5::Test
  KF5::KIOCore
  pimcommon
)

