
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/..)

set(kontact_coisceimplugin_PART_SRCS
  coisceim_plugin.cpp
)

qt5_add_dbus_interfaces(kontact_coisceimplugin_PART_SRCS ${CMAKE_BINARY_DIR}/examples/coisceim/org.kde.coisceim.CoisceimWidget.xml)

add_library(kontact_coisceimplugin MODULE ${kontact_coisceimplugin_PART_SRCS} )

# We need the generated file org.kde.coisceim.CoisceimWidget.xml to exist
add_dependencies(kontact_coisceimplugin coisceim)

target_link_libraries(kontact_coisceimplugin KF5::Parts KF5::KontactInterface)

install(TARGETS kontact_coisceimplugin  DESTINATION ${KDE_INSTALL_PLUGINDIR})

install( FILES coisceim_plugin.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kontact)
