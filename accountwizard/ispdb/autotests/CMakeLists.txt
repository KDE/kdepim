
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )
add_definitions (-DAUTOCONFIG_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
add_definitions (-DCURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

set(ispdbtest_srcs
  ../ispdb.cpp
  ../../accountwizard_debug.cpp
)

set(ispdbtest_libs
  KF5::KIOCore
  KF5::Mime
  Qt5::Test
  Qt5::Xml
  Qt5::Network
  KF5::I18n
)

add_executable(ispdbtest ispdbtest.cpp ${ispdbtest_srcs})
add_test(ispdbtest ispdbtest)
ecm_mark_as_test(ispdbtest)
ecm_mark_nongui_executable(ispdbtest)

target_link_libraries(ispdbtest
  ${ispdbtest_libs}
)

#TODO: Needs to run errorserver.py

set(autoconfigkolabtest_srcs
  ${ispdbtest_srcs}
  ../autoconfigkolabmail.cpp
)

add_executable(autoconfigkolabmailtest autoconfigkolabmailtest.cpp ${autoconfigkolabtest_srcs})
add_test(autoconfigkolabmailtest autoconfigkolabmailtest)
ecm_mark_as_test(autoconfigkolabmailtest)
ecm_mark_nongui_executable(autoconfigkolabmailtest)

target_link_libraries(autoconfigkolabmailtest
  ${ispdbtest_libs}
)

add_executable(autoconfigkolabldaptest autoconfigkolabldaptest.cpp ../autoconfigkolabldap.cpp ${autoconfigkolabtest_srcs})
add_test(autoconfigkolabldaptest autoconfigkolabldaptest)
ecm_mark_as_test(autoconfigkolabldaptest)
ecm_mark_nongui_executable(autoconfigkolabldaptest)

target_link_libraries(autoconfigkolabldaptest
  KF5::Ldap
  ${ispdbtest_libs}
)

add_executable(autoconfigkolabfreebusytest autoconfigkolabfreebusy.cpp ../autoconfigkolabfreebusy.cpp ${autoconfigkolabtest_srcs})
add_test(autoconfigkolabfreebusytest autoconfigkolabfreebusytest)
ecm_mark_as_test(autoconfigkolabfreebusytest)
ecm_mark_nongui_executable(autoconfigkolabfreebusytest)

target_link_libraries(autoconfigkolabfreebusytest
  ${ispdbtest_libs}
)
