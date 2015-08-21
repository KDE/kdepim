

add_definitions(-DTRANSLATION_DOMAIN=\"messageviewer_text_calendar_plugin\")
set(messageviewer_bodypartformatter_text_calendar_PART_SRCS text_calendar.cpp delegateselector.cpp attendeeselector.cpp memorycalendarmemento.cpp syncitiphandler.cpp text_calendar_debug.cpp)

qt5_add_dbus_interfaces(messageviewer_bodypartformatter_text_calendar_PART_SRCS
  ${CMAKE_SOURCE_DIR}/korganizer/data/org.kde.Korganizer.Calendar.xml
)

ki18n_wrap_ui(messageviewer_bodypartformatter_text_calendar_PART_SRCS attendeeselector.ui)

add_library(messageviewer_bodypartformatter_text_calendar MODULE ${messageviewer_bodypartformatter_text_calendar_PART_SRCS})


target_link_libraries(messageviewer_bodypartformatter_text_calendar
  KF5::Codecs
  Qt5::Core 
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::Mime
  KF5::IdentityManagement
  KF5::MailTransport
  KF5::AkonadiMime
  calendarsupport
  kdepim
  mailcommon
  messageviewer
)

target_link_libraries(messageviewer_bodypartformatter_text_calendar incidenceeditorsng)

install(TARGETS messageviewer_bodypartformatter_text_calendar  DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES text_calendar.desktop DESTINATION ${KDE_INSTALL_DATADIR}/messageviewer/plugins/bodypartformatter)
