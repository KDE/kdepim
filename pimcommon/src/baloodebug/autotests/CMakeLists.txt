
macro(add_pimcommon_baloodebug_unittest _source)
    ecm_add_test(${_source}
        NAME_PREFIX "pimcommon-baloodebug-"
        LINK_LIBRARIES Qt5::Test Qt5::Gui KF5::PimCommon KF5::Completion
        GUI
    )
endmacro ()

add_pimcommon_baloodebug_unittest(baloodebugdialogtest.cpp)
add_pimcommon_baloodebug_unittest(baloodebugwidgettest.cpp)
add_pimcommon_baloodebug_unittest(baloodebugsearchpathcomboboxtest.cpp)
