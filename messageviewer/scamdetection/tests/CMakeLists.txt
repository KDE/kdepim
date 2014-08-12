set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_SOURCE_DIR}/messageviewer/
  ${CMAKE_BINARY_DIR}/messageviewer/
)

include(ECMAddTests)
 
set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )
 
set(scamdetection_gui_SRCS scamdetection_gui.cpp)
add_executable(scamdetection_gui ${scamdetection_gui_SRCS})
target_link_libraries(scamdetection_gui
  KF5::KIOCore
  messageviewer
)


