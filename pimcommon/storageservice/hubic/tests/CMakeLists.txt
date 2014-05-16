include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(hubic_gui_SRCS hubic_gui.cpp ../../tests/servicetestwidget.cpp )
add_executable(hubic_gui ${hubic_gui_SRCS})
target_link_libraries(hubic_gui
  Qt5::Test
  KF5::KIOCore
  pimcommon
)

