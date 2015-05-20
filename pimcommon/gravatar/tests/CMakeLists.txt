include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(gravatarwidget_gui_SRCS gravatarwidget_gui.cpp)
add_executable(gravatarwidget_gui ${gravatarwidget_gui_SRCS})
ecm_mark_as_test(gravatarwidget_gui)
target_link_libraries(gravatarwidget_gui
  Qt5::Test
  Qt5::Core
  Qt5::Gui
  
  pimcommon
)

