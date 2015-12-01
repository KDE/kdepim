cmake_minimum_required(VERSION 2.8.12)

project(messagecore)

if(${CMAKE_SOURCE_DIR} STREQUAL ${Messagelib_SOURCE_DIR})

ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX MESSAGECORE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/messagecore_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageCoreConfigVersion.cmake"
                        SOVERSION 5
)


option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)

# config-enterprise.h is needed for both ENTERPRISE_BUILD and BUILD_EVERYTHING
configure_file(src/config-enterprise.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-enterprise.h )

########### Targets ###########

add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5MessageCore")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5MessageCoreConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageCoreConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageCoreConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageCoreConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5MessageCoreTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5MessageCoreTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/messagecore_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

else()
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX MESSAGECORE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/messagecore_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageCoreConfigVersion.cmake"
                        SOVERSION 5
)
endif()

add_subdirectory(src)
if (BUILD_TESTING)
   add_subdirectory(autotests)
endif()

