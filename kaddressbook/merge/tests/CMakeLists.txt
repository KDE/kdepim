set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. )

# Convenience macro to add unit tests.
macro( kaddressbook_merge _source )
  set( _test ${_source}  )
  get_filename_component( _name ${_source} NAME_WE )
  kde4_add_unit_test( ${_name} TESTNAME kaddressbookmerge-${_name} ${_test} )
  target_link_libraries( ${_name} kaddressbookprivate ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_KABC_LIBS} )
endmacro()

kaddressbook_merge(mergecontactwidgettest.cpp)
kaddressbook_merge(mergecontacttest.cpp)


include(AkonadiMacros)
set(KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE)
set(KDEPIMLIBS_RUN_SQLITE_ISOLATED_TESTS TRUE)

add_akonadi_isolated_test_advanced( mergecontactinfowidgettest.cpp "" "kaddressbookprivate")
add_akonadi_isolated_test_advanced( mergecontactshowresultdialogtest.cpp "" "kaddressbookprivate")
add_akonadi_isolated_test_advanced( mergecontactselectinformationdialogtest.cpp "" "kaddressbookprivate")
add_akonadi_isolated_test_advanced( mergecontactduplicatecontactdialogtest.cpp "" "kaddressbookprivate")
add_akonadi_isolated_test_advanced( searchpotentialduplicatecontactjobtest.cpp "" "kaddressbookprivate;kabc")
