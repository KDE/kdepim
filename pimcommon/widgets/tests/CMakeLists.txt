include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(customtextedit_gui_SRCS customtextedit_gui.cpp)
add_executable(customtextedit_gui ${customtextedit_gui_SRCS})
target_link_libraries(customtextedit_gui
  Qt5::Test
  pimcommon
)

set(customtoolswidget_gui_SRCS customtoolswidget_gui.cpp)
add_executable(customtoolswidget_gui ${customtoolswidget_gui_SRCS})
target_link_libraries(customtoolswidget_gui
  Qt5::Test
  pimcommon
)
