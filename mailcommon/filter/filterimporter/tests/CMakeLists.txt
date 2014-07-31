set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(AkonadiMacros)
set(KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE)
set(KDEPIMLIBS_RUN_SQLITE_ISOLATED_TESTS TRUE)

add_akonadi_isolated_test_advanced(filterimportclawmailtest.cpp filtertestkernel.cpp "mailcommon;messagecomposer")
add_akonadi_isolated_test_advanced(filterimportthunderbirdtest.cpp filtertestkernel.cpp "mailcommon;messagecomposer")
add_akonadi_isolated_test_advanced(filterimportbalsatest.cpp filtertestkernel.cpp "mailcommon;messagecomposer")
add_akonadi_isolated_test_advanced(filterimportevolutiontest.cpp filtertestkernel.cpp "mailcommon;messagecomposer")
add_akonadi_isolated_test_advanced(filterimportprocmailtest.cpp filtertestkernel.cpp "mailcommon;messagecomposer")
add_akonadi_isolated_test_advanced(filterimportsylpheedtest.cpp filtertestkernel.cpp "mailcommon;messagecomposer")


# Convenience macro to add messagecomposer unit tests.
#macro( add_filterimport_test _source )
#  set( _test ${_source} filtertestkernel.cpp)
#  get_filename_component( _name ${_source} NAME_WE )
#  kde4_add_unit_test( ${_name} TESTNAME filterimport-${_name} ${_test} )
#  target_link_libraries( ${_name}  mailcommon messagecomposer ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBS})
#endmacro()

#add_filterimport_test( filterimportclawmailtest.cpp )
#add_filterimport_test( filterimportthunderbirdtest.cpp )
#add_filterimport_test( filterimportbalsatest.cpp )
#add_filterimport_test( filterimportevolutiontest.cpp )
#add_filterimport_test( filterimportprocmailtest.cpp )
#add_filterimport_test( filterimportsylpheedtest.cpp )


