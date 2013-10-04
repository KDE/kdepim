kde4_add_executable(sievemanagerdialog TEST sievemanagerdialog.cpp)

target_link_libraries(sievemanagerdialog
  ksieve
  ksieveui
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
)

kde4_add_executable(sievedebugdialog TEST sievedebugdialog.cpp)

target_link_libraries(sievedebugdialog
  ksieve
  ksieveui
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
)
