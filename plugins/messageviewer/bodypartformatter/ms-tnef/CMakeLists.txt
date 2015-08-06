
add_definitions(-DTRANSLATION_DOMAIN=\"messageviewer_application_mstnef_plugin\")

add_library(messageviewer_bodypartformatter_application_mstnef MODULE application_ms-tnef.cpp ms_tnef_debug.cpp)
target_link_libraries(messageviewer_bodypartformatter_application_mstnef
  Qt5::Core
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::Mime
  KF5::Tnef
  messagecore
  messageviewer
)
install(TARGETS messageviewer_bodypartformatter_application_mstnef DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES application_ms-tnef.desktop DESTINATION ${KDE_INSTALL_DATADIR}/messageviewer/plugins/bodypartformatter)
