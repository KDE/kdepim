set(viewertest_gui_SRCS
   viewertest_gui.cpp
   ../messageviewer_debug.cpp
)
add_executable(viewertest_gui ${viewertest_gui_SRCS})
target_link_libraries(viewertest_gui
  Qt5::WebKitWidgets
  KF5::Mime
  messageviewer
  KF5::GrantleeTheme
)

