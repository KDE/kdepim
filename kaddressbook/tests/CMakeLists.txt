kde4_add_unit_test(contactfieldstest NOGUI contactfieldstest.cpp ../contactfields.cpp)
target_link_libraries(contactfieldstest  KF5::Abc Qt5::Test)
