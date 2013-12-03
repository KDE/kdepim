include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(yousendit_gui_SRCS yousendit_gui.cpp )
kde4_add_executable(yousendit_gui TEST ${yousendit_gui_SRCS})
target_link_libraries(yousendit_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  pimcommon
)

