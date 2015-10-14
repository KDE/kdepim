
macro(add_pimcommon_akonadisearchdebug_unittest _source)
    ecm_add_test(${_source}
        NAME_PREFIX "pimcommon-akonadisearchdebug-"
        LINK_LIBRARIES Qt5::Test Qt5::Gui KF5::PimCommon KF5::Completion
        GUI
    )
endmacro ()

add_pimcommon_akonadisearchdebug_unittest(akonadisearchdebugdialogtest.cpp)
add_pimcommon_akonadisearchdebug_unittest(akonadisearchdebugwidgettest.cpp)
add_pimcommon_akonadisearchdebug_unittest(akonadisearchdebugsearchpathcomboboxtest.cpp)
