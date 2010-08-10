########### next target ###############

set(incidenceeditorapp_SRCS incidenceeditorapp.cpp)
qt_wrap_cpp(incidenceeditorapp ${incidenceeditorapp_SRCS} incidenceeditor.cpp)
kde4_add_executable(incidenceeditorapp ${incidenceeditorapp_SRCS})
target_link_libraries(incidenceeditorapp
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KCAL_LIBRARY}
  incidenceeditors
  akonadi-kcal_next
  kdepimdbusinterfaces
  ${KDEPIMLIBS_KCALCORE_LIBS}
)
#install(TARGETS incidenceeditorapp ${INSTALL_TARGETS_DEFAULT_ARGS})

########### KTimeZoneComboBox unit test #############
set(ktimezonecomboboxtest_SRCS
    ktimezonecomboboxtest.cpp
    ../ktimezonecombobox.cpp
)
kde4_add_unit_test(ktimezonecomboboxtest TESTNAME ktimezonecomboboxtest ${ktimezonecomboboxtest_SRCS})

target_link_libraries(
  ktimezonecomboboxtest
  ${KDE4_KDEUI_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
)


