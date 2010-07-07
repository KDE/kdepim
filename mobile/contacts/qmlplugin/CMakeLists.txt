kde4_add_plugin( akonadicontactqmlplugin WITH_PREFIX
  plugin.cpp
  contactviewitem.cpp
  contactgroupviewitem.cpp
)

include_directories(
  ${Boost_INCLUDE_DIR}
)

target_link_libraries( akonadicontactqmlplugin
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBRARY}
  ${QT_QTDECLARATIVE_LIBRARY}
  kdepimmobileui
)

install( FILES qmldir DESTINATION ${PLUGIN_INSTALL_DIR}/imports/org/kde/akonadi/contacts )
install( TARGETS akonadicontactqmlplugin DESTINATION ${PLUGIN_INSTALL_DIR}/imports/org/kde/akonadi/contacts )
