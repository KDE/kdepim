set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
 ../
)

set(resourcemanagement_gui_SRCS resourcemanagment_gui.cpp)
add_executable(resourcemanagement_gui ${resourcemanagement_gui_SRCS})
target_link_libraries(resourcemanagement_gui
  Qt5::Core
  Qt5::Gui
  Qt5::WebKitWidgets
  KF5::Mime
  KF5::IncidenceEditorsng
  KF5::Libkdepim
  KF5::Ldap
)
