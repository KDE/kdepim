macro(add_pimcommon_util_unittest _name)
    ecm_add_test(${ARGN}
        TEST_NAME ${_name}
        NAME_PREFIX "pimcommon-util-"
        LINK_LIBRARIES Qt5::Test Qt5::Gui KF5::PimCommon
        GUI
    )
endmacro()

add_pimcommon_util_unittest(networkutiltest networkutiltest.cpp)
