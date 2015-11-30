set(headerstylemenu_gui_SRCS headerstylemenu_gui.cpp)
add_executable(headerstylemenu_gui ${headerstylemenu_gui_SRCS})
target_link_libraries(headerstylemenu_gui
  KF5::MessageViewer
  KF5::XmlGui
  KF5::GrantleeTheme
)

