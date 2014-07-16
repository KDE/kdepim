include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(storageservice_gui_SRCS storageservice_gui.cpp )
kde4_add_executable(storageservice_gui TEST ${storageservice_gui_SRCS})
target_link_libraries(storageservice_gui
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  pimcommon
)

