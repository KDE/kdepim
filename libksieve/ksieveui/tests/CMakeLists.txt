kde4_add_executable(sievemanager TEST sievemanager.cpp)

target_link_libraries(sievemanager
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
