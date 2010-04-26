include_directories(
  ${CMAKE_SOURCE_DIR}/mobile/lib
)

kde4_add_plugin( calendarviewerqmlplugin WITH_PREFIX
  plugin.cpp
  agendaviewitem.cpp
)

target_link_libraries( calendarviewerqmlplugin
  eventviews
  ${KDE4_KDECORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDEPIMLIBS_AKONADI_LIBRARY}
  ${QT_QTDECLARATIVE_LIBRARY}
  kdepimmobileui
  akonadi-kcal_next
)

install( FILES qmldir DESTINATION ${PLUGIN_INSTALL_DIR}/imports/org/kde/calendarviews )
install( TARGETS calendarviewerqmlplugin DESTINATION ${PLUGIN_INSTALL_DIR}/imports/org/kde/calendarviews )
