set(messageviewer_createtodoplugin_SRCS
    viewerplugincreatetodo.cpp
    viewerplugincreatetodointerface.cpp
)

add_library(messageviewer_createtodoplugin MODULE ${messageviewer_createtodoplugin_SRCS})

target_link_libraries(messageviewer_createtodoplugin
  KF5::MessageViewer KF5::XmlGui
)

install(TARGETS messageviewer_createtodoplugin DESTINATION ${PLUGIN_INSTALL_DIR}/messageviewer)
kcoreaddons_desktop_to_json(messageviewer_createtodoplugin messageviewer_createtodoplugin.desktop)
 
