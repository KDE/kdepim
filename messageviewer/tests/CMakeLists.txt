set(viewertest_gui_SRCS viewertest_gui.cpp)
add_executable(viewertest_gui ${viewertest_gui_SRCS})
target_link_libraries(viewertest_gui
  Qt5::WebKitWidgets
  KF5::Mime
  messageviewer
)

