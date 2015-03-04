set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(imagescaling_gui_SRCS imagescaling_gui.cpp)
add_executable(imagescaling_gui ${imagescaling_gui_SRCS})
target_link_libraries(imagescaling_gui
  Qt5::Test
  Qt5::WebKitWidgets
  KF5::KIOCore
  messagecomposer
)


