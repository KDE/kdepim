cmake_minimum_required(VERSION 2.8.12)

project(MessageViewer)

if(${CMAKE_SOURCE_DIR} STREQUAL ${Messagelib_SOURCE_DIR})

find_package(Grantlee5 "5.0" CONFIG REQUIRED)
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX MESSAGEVIEWER
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/messageviewer_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageViewerConfigVersion.cmake"
                        SOVERSION 5
)


########### Targets ###########

add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")
#verify
remove_definitions("-DQT_NO_URL_CAST_FROM_STRING")
remove_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5MessageViewer")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5MessageViewerConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageViewerConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageViewerConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageViewerConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5MessageViewerTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5MessageViewerTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/messageviewer_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

else()
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX MESSAGEVIEWER
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/messageviewer_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageViewerConfigVersion.cmake"
                        SOVERSION 5
)
endif()

add_subdirectory(src)
if (BUILD_TESTING)
   add_subdirectory(autotests)
   add_subdirectory(tests)
endif()

