set(messageviewer_translatorplugin_SRCS
    viewerplugintranslator.cpp
    viewerplugintranslatorinterface.cpp
)

add_library(messageviewer_translatorplugin MODULE ${messageviewer_translatorplugin_SRCS})

target_link_libraries(messageviewer_translatorplugin
  KF5::MessageViewer KF5::XmlGui KF5::PimTextEdit
)

install(TARGETS messageviewer_translatorplugin DESTINATION ${PLUGIN_INSTALL_DIR}/messageviewer)
kcoreaddons_desktop_to_json(messageviewer_translatorplugin messageviewer_translatorplugin.desktop)
 
