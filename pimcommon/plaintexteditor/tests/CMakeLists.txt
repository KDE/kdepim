include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(plaintexteditor_gui_SRCS plaintexteditor_gui.cpp)
kde4_add_executable(plaintexteditor_gui TEST ${plaintexteditor_gui_SRCS})
target_link_libraries(plaintexteditor_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  pimcommon
)

