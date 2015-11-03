macro(add_pimcommon_autocorrect_unittest _source)
    ecm_add_test(${_source}
        NAME_PREFIX "pimcommon-autocorrection-"
        LINK_LIBRARIES Qt5::Test KF5::PimCommon KF5::PimTextEdit
    )
endmacro()

add_pimcommon_autocorrect_unittest(autocorrectiontest.cpp)
add_pimcommon_autocorrect_unittest(lineeditwithautocorrectiontest.cpp)
add_pimcommon_autocorrect_unittest(richtexteditwithautocorrectiontest.cpp)
