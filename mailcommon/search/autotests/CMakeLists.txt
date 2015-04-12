set( mailcommon_searchpatterntest_source searchpatterntest.cpp )
kde4_add_unit_test( searchpatterntest ${mailcommon_searchpatterntest_source})
target_link_libraries( searchpatterntest
  mailcommon
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${QT_QTGUI_LIBRARY}
)

set( mailcommon_searchruletest_source searchruletest.cpp )
kde4_add_unit_test( searchruletest ${mailcommon_searchruletest_source})
target_link_libraries( searchruletest
  mailcommon
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${QT_QTGUI_LIBRARY}
)

set( mailcommon_searchrulestatustest_source searchrulestatustest.cpp )
kde4_add_unit_test( searchrulestatustest ${mailcommon_searchrulestatustest_source})
target_link_libraries( searchrulestatustest
  mailcommon
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${QT_QTGUI_LIBRARY}
)
