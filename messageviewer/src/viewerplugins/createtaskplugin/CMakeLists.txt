set(messageviewer_createtaskplugin_SRCS
    viewerplugincreatetask.cpp
    viewerplugincreatetaskinterface.cpp
)

add_library(messageviewer_createtaskplugin MODULE ${messageviewer_createtaskplugin_SRCS})

target_link_libraries(messageviewer_createtaskplugin
  KF5::MessageViewer KF5::XmlGui KF5::CalendarCore
)

install(TARGETS messageviewer_createtaskplugin DESTINATION ${PLUGIN_INSTALL_DIR}/messageviewer)
kcoreaddons_desktop_to_json(messageviewer_createtaskplugin messageviewer_createtaskplugin.desktop)
 
