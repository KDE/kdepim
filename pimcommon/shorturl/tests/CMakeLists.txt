include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(shorturlwidget_gui_SRCS shorturlwidget_gui.cpp shorturlmainwidget.cpp)
add_executable(shorturlwidget_gui ${shorturlwidget_gui_SRCS})
target_link_libraries(shorturlwidget_gui
  Qt5::Test
  pimcommon
)

