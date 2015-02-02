include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon
)

set( pimcommon_imapaclattributetest_source imapaclattributetest.cpp ../imapaclattribute.cpp )
add_executable( imapaclattributetest ${pimcommon_imapaclattributetest_source})
add_test(imapaclattributetest imapaclattributetest)
ecm_mark_as_test(imapaclattributetest)
target_link_libraries( imapaclattributetest Qt5::Test Qt5::Gui KF5::AkonadiCore  KF5::Imap mailcommon)

set( pimcommon_collectionaclwidgettest_source collectionaclwidgettest.cpp ../collectionaclwidget.cpp )
add_executable( collectionaclwidgettest ${pimcommon_collectionaclwidgettest_source})
add_test(collectionaclwidgettest collectionaclwidgettest)
ecm_mark_as_test(collectionaclwidgettest)
target_link_libraries( collectionaclwidgettest Qt5::Test KF5::KDELibs4Support Qt5::Gui KF5::AkonadiCore  KF5::Imap pimcommon)

