set(headerstylemenu_gui_SRCS headerstylemenu_gui.cpp)
add_executable(headerstylemenu_gui ${headerstylemenu_gui_SRCS})
target_link_libraries(headerstylemenu_gui
  Qt5::Test
  messageviewer
  KF5::XmlGui
)

