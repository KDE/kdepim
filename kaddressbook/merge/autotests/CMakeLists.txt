set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. )

# Convenience macro to add unit tests.
macro( kaddressbook_merge _source )
  set( _test ${_source}  )
  get_filename_component( _name ${_source} NAME_WE )
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(kaddressbookmerge-${_name})
  target_link_libraries( ${_name} kaddressbookprivate Qt5::Test  KF5::AkonadiCore KF5::Contacts )
endmacro()

kaddressbook_merge(mergecontactwidgettest.cpp)
kaddressbook_merge(mergecontacttest.cpp)
kaddressbook_merge(mergecontactjobtest.cpp)

set(KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE)
set(KDEPIMLIBS_RUN_SQLITE_ISOLATED_TESTS TRUE)

add_akonadi_isolated_test_advanced( mergecontactinfowidgettest.cpp "" "kaddressbookprivate")
add_akonadi_isolated_test_advanced( mergecontactshowresultdialogtest.cpp "" "kaddressbookprivate;KF5::Contacts")
add_akonadi_isolated_test_advanced( mergecontactselectinformationdialogtest.cpp "" "kaddressbookprivate")
add_akonadi_isolated_test_advanced( searchandmergecontactduplicatecontactdialogtest.cpp "" "kaddressbookprivate")
add_akonadi_isolated_test_advanced( searchpotentialduplicatecontactjobtest.cpp "" "kaddressbookprivate;KF5::Contacts")
