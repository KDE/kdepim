include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_BINARY_DIR}/accountwizard )
add_definitions (-DTEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
add_definitions (-DCURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(ldaptest ldaptest.cpp ../ldap.cpp ../setupobject.cpp)
add_test(ldaptest ldaptest)
ecm_mark_as_test(ldaptest)

target_link_libraries(ldaptest
    KF5::KIOCore
    KF5::Mime
    KF5::Ldap
    Qt5::Test
    KF5::Libkdepim
    KF5::I18n
    )
