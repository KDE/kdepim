include_directories( 
                    ${CMAKE_SOURCE_DIR}/pimcommon/src/customtools/shorturlplugin/
)

set(shorturlenginestest_SRCS
   ../abstractshorturl.cpp
   ../shorturlwidget.cpp
   ../engine/tinyurlshorturl.cpp
   ../engine/googleshorturl.cpp
   ../shorturlconfigurewidget.cpp
   ../shorturlutils.cpp
   ../shorturlconfiguredialog.cpp
   ../engine/migremeshorturl.cpp
   ../engine/triopabshorturl.cpp
   ../engine/ur1cashorturl.cpp
   ../engine/isgdshorturl.cpp
)

ecm_add_test(shorturlenginestest.cpp ${shorturlenginestest_SRCS}
    TEST_NAME shorturlenginestest
    NAME_PREFIX "pimcommon-shorturl-"
    LINK_LIBRARIES Qt5::Test Qt5::Gui KF5::PimCommon KF5::Completion KF5::KIOWidgets KF5::Libkdepim KF5::XmlGui
    GUI
)
