include(ECMAddTests)

set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)
include_directories(../)

########### next target ###############

ecm_add_tests(testaddresseelineedit.cpp kcheckcomboboxtest_gui.cpp testldapclient.cpp testspellchecklineedit.cpp NAME_PREFIX "libkdepim-" LINK_LIBRARIES kdepim Qt5::Test)

