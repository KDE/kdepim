ecm_add_test(kactionmenuchangecasetest.cpp ../kactionmenuchangecase.cpp
    TEST_NAME kactionmenuchangecasetest
    NAME_PREFIX pimcommon-widget
    LINK_LIBRARIES Qt5::Test Qt5::Gui KF5::WidgetsAddons KF5::I18n KF5::XmlGui KF5::PimCommon
)
