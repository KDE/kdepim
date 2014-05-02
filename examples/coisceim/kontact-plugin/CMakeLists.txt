
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/..)

set(kontact_coisceimplugin_PART_SRCS
  coisceim_plugin.cpp
)

qt5_add_dbus_interfaces(kontact_coisceimplugin_PART_SRCS ${CMAKE_BINARY_DIR}/examples/coisceim/org.kde.coisceim.CoisceimWidget.xml)

kde4_add_plugin(kontact_coisceimplugin ${kontact_coisceimplugin_PART_SRCS} )

# We need the generated file org.kde.coisceim.CoisceimWidget.xml to exist
add_dependencies(kontact_coisceimplugin coisceim)

target_link_libraries(kontact_coisceimplugin ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KONTACTINTERFACE_LIBS})

install(TARGETS kontact_coisceimplugin  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES coisceim_plugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
