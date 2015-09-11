ecm_add_test(accessiblemailwebviewtest.cpp
    NAME_PREFIX "KF5::MessageViewer-a11y-"
    LINK_LIBRARIES Qt5::Test Qt5::WebKitWidgets KF5::WebKit KF5::MessageViewer
)
