add_executable( contactfieldstest contactfieldstest.cpp ../contactfields.cpp )
add_test( contactfieldstest contactfieldstest )
ecm_mark_as_test(kaddrebook-contactfieldstest)
target_link_libraries(contactfieldstest  KF5::Abc Qt5::Test)
