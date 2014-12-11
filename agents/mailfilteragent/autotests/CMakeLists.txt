
set( mailfilteragent_pop3resourceattributetest_source pop3resourceattributetest.cpp ../pop3resourceattribute.cpp)
add_executable( pop3resourceattributetest ${mailfilteragent_pop3resourceattributetest_source})
add_test(pop3resourceattributetest pop3resourceattributetest)
ecm_mark_as_test(pop3resourceattributetest)
target_link_libraries( pop3resourceattributetest Qt5::Test KF5::KDELibs4Support Qt5::Gui KF5::AkonadiCore)

