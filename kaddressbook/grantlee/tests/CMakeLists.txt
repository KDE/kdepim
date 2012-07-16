kde4_add_unit_test(thememanagertest
  NOGUI
  thememanagertest.cpp
  ../thememanager.cpp
  ../theme.cpp
)
target_link_libraries(thememanagertest ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})
