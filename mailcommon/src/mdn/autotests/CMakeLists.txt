ecm_add_test(mdnstateattributetest.cpp ../mdnstateattribute.cpp
    TEST_NAME mdnattributetest
    NAME_PREFIX "mailcommon-mdn-"
    LINK_LIBRARIES Qt5::Test Qt5::Gui KF5::AkonadiCore KF5::MailCommon
)
