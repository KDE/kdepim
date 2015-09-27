set(messageviewer_createeventplugin_SRCS
    viewerplugincreateevent.cpp
    viewerplugincreateeventinterface.cpp
)

add_library(messageviewer_createeventplugin MODULE ${messageviewer_createeventplugin_SRCS})

target_link_libraries(messageviewer_createeventplugin
  KF5::MessageViewer KF5::XmlGui
)

install(TARGETS messageviewer_createeventplugin DESTINATION ${PLUGIN_INSTALL_DIR}/messageviewer)
kcoreaddons_desktop_to_json(messageviewer_createeventplugin messageviewer_createeventplugin.desktop)
 
