set(messageviewer_createeventplugin_SRCS
    viewerplugincreateevent.cpp
    viewerplugincreateeventinterface.cpp
    eventedit.cpp
    eventdatetimewidget.cpp
)

ecm_qt_declare_logging_category(messageviewer_createeventplugin_SRCS HEADER createeventplugin_debug.h IDENTIFIER CREATEEVENTPLUGIN_LOG CATEGORY_NAME log_createeventplugin)

add_library(messageviewer_createeventplugin MODULE ${messageviewer_createeventplugin_SRCS})

target_link_libraries(messageviewer_createeventplugin
  KF5::MessageViewer KF5::XmlGui KF5::CalendarCore KF5::IncidenceEditorsng
)

install(TARGETS messageviewer_createeventplugin DESTINATION ${PLUGIN_INSTALL_DIR}/messageviewer)
kcoreaddons_desktop_to_json(messageviewer_createeventplugin messageviewer_createeventplugin.desktop)
 
