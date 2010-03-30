if (QT_QTDECLARATIVE_FOUND)

  kde4_add_plugin( messageviewerqmlplugin WITH_PREFIX
    plugin.cpp
    messageviewitem.cpp
  )
  target_link_libraries(messageviewerqmlplugin messageviewer
    ${KDE4_KDECORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDEPIMLIBS_KMIME_LIBRARY}
    ${KDEPIMLIBS_AKONADI_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    kdepim
  )

  install( FILES qmldir DESTINATION ${PLUGIN_INSTALL_DIR}/imports/org/kde/messageviewer )
  install( TARGETS messageviewerqmlplugin DESTINATION ${PLUGIN_INSTALL_DIR}/imports/org/kde/messageviewer )

  add_subdirectory(tests)
endif (QT_QTDECLARATIVE_FOUND)
