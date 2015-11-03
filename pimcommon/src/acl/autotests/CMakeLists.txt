ecm_add_test(imapaclattributetest.cpp ../imapaclattribute.cpp
    TEST_NAME imapaclattributetest
    NAME_PREFIX "pimcommon-acl-"
    LINK_LIBRARIES Qt5::Test Qt5::Gui KF5::AkonadiCore KF5::IMAP KF5::PimCommon
    GUI
)

ecm_add_test(collectionaclwidgettest.cpp ../collectionaclwidget.cpp
    TEST_NAME collectionaclwidgettest
    NAME_PREFIX "pimcommon-acl-"
    LINK_LIBRARIES Qt5::Test Qt5::Gui KF5::AkonadiCore KF5::IMAP KF5::PimCommon
)
