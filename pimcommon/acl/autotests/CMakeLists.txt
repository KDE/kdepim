include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon
)

set( pimcommon_imapaclattributetest_source imapaclattributetest.cpp ../imapaclattribute.cpp )
add_executable( imapaclattributetest ${pimcommon_imapaclattributetest_source})
add_test(imapaclattributetest imapaclattributetest)
ecm_mark_as_test(imapaclattributetest)
target_link_libraries( imapaclattributetest Qt5::Test Qt5::Gui KF5::AkonadiCore  KF5::Imap mailcommon)
