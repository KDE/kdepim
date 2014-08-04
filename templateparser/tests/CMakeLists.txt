include(ECMAddTests)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/libkleo
  ${CMAKE_SOURCE_DIR}/templateparser/
  ${CMAKE_BINARY_DIR}/templateparser/
)

set(templateeditor_gui_SRCS templateeditor_gui.cpp)
add_executable(templateeditor_gui ${templateeditor_gui_SRCS})
target_link_libraries(templateeditor_gui
  Qt5::Test
  Qt5::WebKitWidgets
  KF5::KIOCore
  templateparser
)

set(templateconfigurewidget_gui_SRCS templateconfigurewidget_gui.cpp)
add_executable(templateconfigurewidget_gui ${templateconfigurewidget_gui_SRCS})
target_link_libraries(templateconfigurewidget_gui
  Qt5::Test
  Qt5::WebKitWidgets
  KF5::KIOCore
  templateparser
)
