cmake_minimum_required(VERSION 2.8.12)

project(TemplateParser)


if(${CMAKE_SOURCE_DIR} STREQUAL ${Messagelib_SOURCE_DIR})
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX TEMPLATEPARSER
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/templateparser_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5TemplateParserConfigVersion.cmake"
                        SOVERSION 5
)

remove_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5TemplateParser")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5TemplateParserConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5TemplateParserConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5TemplateParserConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5TemplateParserConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5TemplateParserTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5TemplateParserTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/templateparser_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

else()
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX TEMPLATEPARSER
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/mailimporter_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5MailImporterConfigVersion.cmake"
                        SOVERSION 5
)
endif()

add_subdirectory(src)
if (BUILD_TESTING)
   add_subdirectory(autotests)
   add_subdirectory(tests)
endif()

