ecm_add_test(findbarbasetest.cpp  ../findbarbase.cpp
    TEST_NAME findbarbasetest
    NAME_PREFIX "messageviewer-findbar-"
    LINK_LIBRARIES Qt5::Test Qt5::Widgets Qt5::Gui KF5::Completion KF5::I18n KF5::ConfigWidgets KF5::PimCommon
)
set_property(TARGET findbarbasetest PROPERTY INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/messageviewer)
