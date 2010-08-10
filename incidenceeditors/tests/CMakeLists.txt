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


