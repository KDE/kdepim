include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(richtexteditor_gui_SRCS richtexteditor_gui.cpp)
kde4_add_executable(richtexteditor_gui TEST ${richtexteditor_gui_SRCS})
target_link_libraries(richtexteditor_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  pimcommon
)

