macro(add_libkdepim_freebusymodel_unittest _name)
    ecm_add_test(${_name}.cpp modeltest.cpp
        TEST_NAME ${_name}
        NAME_PREFIX "libkdepim-freebusymodel-"
        LINK_LIBRARIES Qt5::Test KF5::AkonadiCore KF5::CalendarUtils KF5::CalendarCore KF5::CalendarSupport
    )
endmacro()

add_libkdepim_freebusymodel_unittest(testfreeperiodmodel)
add_libkdepim_freebusymodel_unittest(testfreebusyitemmodel)
