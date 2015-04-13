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

set( mailcommon_searchrulenumericaltest_source searchrulenumericaltest.cpp )
kde4_add_unit_test( searchrulenumericaltest ${mailcommon_searchrulenumericaltest_source})
target_link_libraries( searchrulenumericaltest
  mailcommon
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${QT_QTGUI_LIBRARY}
)

set( mailcommon_searchruledatetest_source searchruledatetest.cpp )
kde4_add_unit_test( searchruledatetest ${mailcommon_searchruledatetest_source})
target_link_libraries( searchruledatetest
  mailcommon
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${QT_QTGUI_LIBRARY}
)

set( mailcommon_searchrulestringtest_source searchrulestringtest.cpp )
kde4_add_unit_test( searchrulestringtest ${mailcommon_searchrulestringtest_source})
target_link_libraries( searchrulestringtest
  mailcommon
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${QT_QTGUI_LIBRARY}
)
