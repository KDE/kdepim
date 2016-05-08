# convenience macro to add qtest unit tests
macro(pimcommon_widget_unittest _source)
    get_filename_component(_name ${_source} NAME_WE)
    ecm_add_test(${_source}
        TEST_NAME ${_name}
        NAME_PREFIX "pimcommon_widget"
        LINK_LIBRARIES Qt5::Test Qt5::Gui KF5::WidgetsAddons KF5::I18n KF5::XmlGui KF5::PimCommon KF5::Completion
    )
endmacro ()

pimcommon_widget_unittest(kactionmenuchangecasetest.cpp)
pimcommon_widget_unittest(lineeditwithkeywordstest.cpp)
