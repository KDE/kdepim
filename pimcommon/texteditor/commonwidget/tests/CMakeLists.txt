include(ECMAddTests)
 
set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)
include_directories( ${CMAKE_SOURCE_DIR}/pimcommon/ )
 
ecm_add_tests(textgotolinewidgettest.cpp NAME_PREFIX "pimcommon-plaintext-" LINK_LIBRARIES pimcommon Qt5::Test)


