ecm_add_test(test_scamdetection.cpp
    TEST_NAME scamdetectiontest
    NAME_PREFIX "messageviewer-scamdetection-"
    LINK_LIBRARIES messageviewer Qt5::Test
)

ecm_add_test(scamattributetest.cpp ../scamattribute.cpp
    TEST_NAME scamattributetest
    NAME_PREFIX "messageviewer-scamdetection-"
    LINK_LIBRARIES Qt5::Test KF5::AkonadiCore
)
