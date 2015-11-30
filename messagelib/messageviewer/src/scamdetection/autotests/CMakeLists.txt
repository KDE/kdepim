ecm_add_test(test_scamdetection.cpp
    TEST_NAME scamdetectiontest
    NAME_PREFIX "messageviewer-scamdetection-"
    LINK_LIBRARIES KF5::MessageViewer Qt5::Test Qt5::WebKitWidgets
)

ecm_add_test(scamattributetest.cpp ../scamattribute.cpp
    TEST_NAME scamattributetest
    NAME_PREFIX "messageviewer-scamdetection-"
    LINK_LIBRARIES Qt5::Test KF5::AkonadiCore
)
