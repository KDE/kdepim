ecm_add_test(imagescalingtest.cpp
    NAME_PREFIX "messagecomposer-imagescaling-"
    LINK_LIBRARIES KF5::Mime KF5::MessageComposer KF5::MessageViewer KF5::MessageCore KF5::TemplateParser
                   KF5::PimTextEdit Qt5::Test KF5::IdentityManagement KF5::Contacts
                   KF5::AkonadiCore
)
