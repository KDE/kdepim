
add_definitions(-DTRANSLATION_DOMAIN=\"messageviewer_application_mstnef_plugin\")

set(messageviewer_bodypartformatter_application_mstnef_SRCS application_ms-tnef.cpp)
ecm_qt_declare_logging_category(messageviewer_bodypartformatter_application_mstnef_SRCS HEADER ms_tnef_debug.h IDENTIFIER MS_TNEF_LOG CATEGORY_NAME log_ms_tnef)

add_library(messageviewer_bodypartformatter_application_mstnef MODULE ${messageviewer_bodypartformatter_application_mstnef_SRCS})

target_link_libraries(messageviewer_bodypartformatter_application_mstnef
  Qt5::Core
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::Mime
  KF5::Tnef
  KF5::MessageCore
  KF5::MessageViewer
)
install(TARGETS messageviewer_bodypartformatter_application_mstnef DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES application_ms-tnef.desktop DESTINATION ${KDE_INSTALL_DATADIR}/messageviewer/plugins/bodypartformatter)
