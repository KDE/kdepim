set( libksieveui_sieveeditormenubartest_source sieveeditormenubartest.cpp ../sieveeditormenubar.cpp )
add_executable( sieveeditormenubartest ${libksieveui_sieveeditormenubartest_source})
add_test(sieveeditormenubartest sieveeditormenubartest)
ecm_mark_as_test(sieveeditormenubartest)
target_link_libraries( sieveeditormenubartest Qt5::Test KF5::KDELibs4Support Qt5::Gui )


