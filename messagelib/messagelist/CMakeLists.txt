cmake_minimum_required(VERSION 2.8.12)

project(MessageList)


if(${CMAKE_SOURCE_DIR} STREQUAL ${Messagelib_SOURCE_DIR})

ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX MESSAGELIST
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/messagelist_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageListConfigVersion.cmake"
                        SOVERSION 5
)

########### Targets ###########

#add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5MessageList")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5MessageListConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageListConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageListConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageListConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5MessageListTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5MessageListTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/messagelist_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
else()
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX MESSAGELIST
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/messagelist_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5MailImporterConfigVersion.cmake"
                        SOVERSION 5
)
endif()

add_subdirectory(src)
if (BUILD_TESTING)
   add_subdirectory(autotests)
endif()

