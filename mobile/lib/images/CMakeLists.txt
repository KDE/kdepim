add_subdirectory(status)

FILE(GLOB IMAGES *.png)
install(FILES ${IMAGES}
              DESTINATION ${PLUGIN_INSTALL_DIR}/imports/org/kde/pim/mobileui/images)
