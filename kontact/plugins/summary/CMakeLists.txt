project(summary_applet)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

include_directories(${CMAKE_SOURCE_DIR}/libkdepimdbusinterfaces)
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories( /usr/include /usr/include/kde4 )

########### next target ###############

set(kontact_summaryplugin_PART_SRCS summaryview_plugin.cpp )

if(KMAIL_SUPPORTED)
   qt4_add_dbus_interfaces(kontact_summaryplugin_PART_SRCS ${CMAKE_BINARY_DIR}/kmail/org.kde.kmail.kmail.xml)
endif(KMAIL_SUPPORTED)

kde4_add_plugin(kontact_summaryplugin ${kontact_summaryplugin_PART_SRCS})
if(KMAIL_SUPPORTED)
   add_dependencies(kontact_summaryplugin kmail_xml)
endif(KMAIL_SUPPORTED)

## Check if the kpart is installed? ##

target_link_libraries(kontact_summaryplugin ${KDE4_KCMUTILS_LIBS} ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBS} ${KDEPIMLIBS_KONTACTINTERFACE_LIBS} ${KDE4_PLASMA_LIBS} kdepim kontactinterface kdepimdbusinterfaces)

## build applets ##
add_subdirectory( applets )

########### install files ###############

install(TARGETS kontact_summaryplugin  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES summaryplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install(FILES kontactsummary.rc  DESTINATION ${DATA_INSTALL_DIR}/kontact_summaryplugin)

install(FILES plasma-default-layout.js DESTINATION ${DATA_INSTALL_DIR}/kontact/plasma/layout/init/)
install(FILES summary.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)
