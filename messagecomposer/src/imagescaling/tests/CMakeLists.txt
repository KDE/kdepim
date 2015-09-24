set(imagescaling_gui_SRCS imagescaling_gui.cpp)
add_executable(imagescaling_gui ${imagescaling_gui_SRCS})
target_link_libraries(imagescaling_gui
  Qt5::WebKitWidgets
  KF5::KIOCore
  KF5::MessageComposer
)


