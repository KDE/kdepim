set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
add_definitions( -DKDELIBS4CONFIGMIGRATER_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data" )

ecm_add_tests( kdelibs4configmigratertest.cpp  NAME_PREFIX "pimcommon-utils-" LINK_LIBRARIES pimcommon Qt5::Test)

