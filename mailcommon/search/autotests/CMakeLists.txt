set( mailcommon_searchpatterntest_source searchpatterntest.cpp )
kde4_add_unit_test( searchpatterntest ${mailcommon_searchpatterntest_source})
target_link_libraries( searchpatterntest
  mailcommon
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${QT_QTGUI_LIBRARY}
)
