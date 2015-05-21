set( mailcommon_searchpatterntest_source searchpatterntest.cpp )
add_executable( searchpatterntest ${mailcommon_searchpatterntest_source})
add_test(searchpatterntest searchpatterntest)
ecm_mark_as_test(searchpatterntest)
target_link_libraries( searchpatterntest
  mailcommon
  Qt5::Test
  Qt5::Gui
)

set( mailcommon_searchruletest_source searchruletest.cpp )
add_executable( searchruletest ${mailcommon_searchruletest_source})
add_test(searchruletest searchruletest)
ecm_mark_as_test(searchruletest)
target_link_libraries( searchruletest
  mailcommon
  Qt5::Test
  Qt5::Gui
)

set( mailcommon_searchrulestatustest_source searchrulestatustest.cpp ../searchrule/searchrulestatus.cpp )
add_executable( searchrulestatustest ${mailcommon_searchrulestatustest_source})
add_test(searchrulestatustest searchrulestatustest)
ecm_mark_as_test(searchrulestatustest)
target_link_libraries( searchrulestatustest
  mailcommon
  Qt5::Test
  Qt5::Gui
  KF5::Mime
  KF5::AkonadiCore
  KF5::AkonadiMime
)

set( mailcommon_searchrulenumericaltest_source searchrulenumericaltest.cpp ../searchrule/searchrulenumerical.cpp )
add_executable( searchrulenumericaltest ${mailcommon_searchrulenumericaltest_source})
add_test(searchrulenumericaltest searchrulenumericaltest)
ecm_mark_as_test(searchrulenumericaltest)
target_link_libraries( searchrulenumericaltest
  mailcommon
  Qt5::Test
  Qt5::Gui
  KF5::Mime
  KF5::AkonadiCore
)

set( mailcommon_searchruledatetest_source searchruledatetest.cpp ../searchrule/searchruledate.cpp)
add_executable( searchruledatetest ${mailcommon_searchruledatetest_source})
add_test(searchruledatetest searchruledatetest)
ecm_mark_as_test(searchruledatetest)
target_link_libraries( searchruledatetest
  mailcommon
  Qt5::Test
  Qt5::Gui
  KF5::Mime
  KF5::AkonadiCore
)

set( mailcommon_searchrulestringtest_source searchrulestringtest.cpp ../searchrule/searchrulestring.cpp)
add_executable( searchrulestringtest ${mailcommon_searchrulestringtest_source})
add_test(searchrulestringtest searchrulestringtest)
ecm_mark_as_test(searchrulestringtest)
target_link_libraries( searchrulestringtest
  mailcommon
  Qt5::Test
  Qt5::Gui
  KF5::AkonadiContact
  KF5::Mime
  KF5::AkonadiCore
)
