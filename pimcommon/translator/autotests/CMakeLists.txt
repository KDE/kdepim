include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)

ecm_add_tests( translatortest.cpp  NAME_PREFIX "pimcommon-" LINK_LIBRARIES pimcommon Qt5::Test)
