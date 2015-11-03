set(viewerplugin_gui_SRCS viewerplugin_gui.cpp)
add_executable(viewerplugin_gui ${viewerplugin_gui_SRCS})
target_link_libraries(viewerplugin_gui
  KF5::MessageViewer
  KF5::XmlGui
)

