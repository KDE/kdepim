ecm_add_test(accessiblemailwebviewtest.cpp
    NAME_PREFIX "messageviewer-a11y-"
    LINK_LIBRARIES Qt5::Test Qt5::WebKitWidgets KF5::WebKit messageviewer
)
