cmake_minimum_required(VERSION 2.8.12)

project(Messagelib)

if(${CMAKE_SOURCE_DIR} STREQUAL ${Messagelib_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${MessageLib_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
endif()

add_subdirectory(messageviewer)
add_subdirectory(templateparser)
add_subdirectory(messagecomposer)
add_subdirectory(messagecore)
add_subdirectory(messagelist)

install( FILES messagelib.categories DESTINATION ${KDE_INSTALL_CONFDIR} )
