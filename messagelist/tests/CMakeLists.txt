include(ECMAddTests)

set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########### next target ###############

ecm_add_tests(itemsizetest.cpp quicksearchlinetest.cpp NAME_PREFIX "messagelist-" LINK_LIBRARIES messagecore messagelist Qt5::Test)
