cmake_minimum_required(VERSION 2.8.12)

project(kdepimdbusinterfaces)

if(${CMAKE_SOURCE_DIR} STREQUAL ${kdepim-apps-lib_SOURCE_DIR})
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX KDEPIMDBUSINTERFACES
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdepimdbusinterfaces_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KdepimDBusInterfacesConfigVersion.cmake"
                        SOVERSION 5
)

########### Targets ###########

add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")
remove_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )
########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5KdepimDBusInterfaces")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5KdepimDBusInterfacesConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KdepimDBusInterfacesConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KdepimDBusInterfacesConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KdepimDBusInterfacesConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5KdepimDBusInterfacesTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5KdepimDBusInterfacesTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kdepimdbusinterfaces_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

else()
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX KDEPIMDBUSINTERFACES
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdepimdbusinterfaces_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KdepimDBusInterfacesConfigVersion.cmake"
                        SOVERSION 5
)
endif()

add_subdirectory(src)

