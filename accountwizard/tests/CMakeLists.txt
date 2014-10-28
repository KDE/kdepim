include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )
add_definitions (-DTEST_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\"")
add_definitions (-DCURRENT_SOURCE_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}\\"")

kde4_add_unit_test(ldaptest ldaptest.cpp ../ldap.cpp ../setupobject.cpp)

target_link_libraries(ldaptest
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${KDEPIMLIBS_KLDAP_LIBS}
    ${QT_QTTEST_LIBRARY}
    kdepim
  )