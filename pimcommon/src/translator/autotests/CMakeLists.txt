ecm_add_test(translatortest.cpp
    NAME_PREFIX "pimcommon-translator-"
    LINK_LIBRARIES KF5::PimCommon Qt5::Test KF5::XmlGui KF5::PimTextEdit
)
