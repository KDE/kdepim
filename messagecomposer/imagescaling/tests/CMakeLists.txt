set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_SOURCE_DIR}/messagecomposer
  ${CMAKE_BINARY_DIR}/messagecomposer
)

set(imagescaling_gui_SRCS imagescaling_gui.cpp)
kde4_add_executable(imagescaling_gui TEST ${imagescaling_gui_SRCS})
target_link_libraries(imagescaling_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${QT_QTWEBKIT_LIBRARY}
  ${KDE4_KIO_LIBS}
  messagecomposer
)


