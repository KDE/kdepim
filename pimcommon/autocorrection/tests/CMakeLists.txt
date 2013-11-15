set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon
  ${CMAKE_BINARY_DIR}/pimcommon
)

set(autocorrection_gui_SRCS autocorrection_gui.cpp)
kde4_add_executable(autocorrection_gui TEST ${autocorrection_gui_SRCS})
target_link_libraries(autocorrection_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${QT_QTWEBKIT_LIBRARY}
  ${KDE4_KIO_LIBS}
  pimcommon
)


set(richtexteditwithautocorrection_gui_SRCS richtexteditwithautocorrection_gui.cpp)
kde4_add_executable(richtexteditwithautocorrection_gui TEST ${richtexteditwithautocorrection_gui_SRCS})
target_link_libraries(richtexteditwithautocorrection_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${QT_QTWEBKIT_LIBRARY}
  ${KDE4_KIO_LIBS}
  pimcommon
)

