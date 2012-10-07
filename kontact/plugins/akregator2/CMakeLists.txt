
include_directories(
  ${CMAKE_SOURCE_DIR}/akregator2/src
  ${Boost_INCLUDE_DIRS}
)

########### next target ###############

set(kontact_akregator2_PART_SRCS akregator_plugin.cpp)

qt4_add_dbus_interfaces(kontact_akregator2_PART_SRCS ${CMAKE_SOURCE_DIR}/akregator2/src/org.kde.akregator2.part.xml)

kde4_add_plugin(kontact_akregator2plugin ${kontact_akregator2_PART_SRCS})

target_link_libraries(kontact_akregator2plugin ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KONTACTINTERFACE_LIBS})

########### install files ###############

install(TARGETS kontact_akregator2plugin DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES akregator2plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

install(FILES akregator2.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)

