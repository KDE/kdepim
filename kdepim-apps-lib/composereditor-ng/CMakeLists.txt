
cmake_minimum_required(VERSION 2.8.12)

project(composereditorng)

if(${CMAKE_SOURCE_DIR} STREQUAL ${kdepim-apps-lib_SOURCE_DIR})


ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX COMPOSEREDITORNG
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/composereditorng_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5ComposerEditorNGConfigVersion.cmake"
                        SOVERSION 5
)

########### Targets ###########

add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5ComposerEditorNG")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5ComposerEditorNGConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5ComposerEditorNGConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5ComposerEditorNGConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5ComposerEditorNGConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5ComposerEditorNGTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5ComposerEditorNGTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/composereditorng_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)
else()
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX COMPOSEREDITORNG
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/composereditorng_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5ComposerEditorNGConfigVersion.cmake"
                        SOVERSION 5
)
endif()

# Laurent install it if you want to debug it
#add_subdirectory(tests)
add_subdirectory(src)
