add_subdirectory(status)

FILE(GLOB IMAGES *.png)
install(FILES ${IMAGES}
              DESTINATION ${KDE_INSTALL_PLUGINDIR}/imports/org/kde/pim/mobileui/images)
