set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMAddTests)
 
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )
 
set(scamdetection_gui_SRCS scamdetection_gui.cpp)
add_executable(scamdetection_gui ${scamdetection_gui_SRCS})
target_link_libraries(scamdetection_gui
  KF5::KIOCore
  KF5::MessageViewer
  Qt5::WebKitWidgets
)




