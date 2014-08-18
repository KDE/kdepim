
include_directories(
  ${CMAKE_SOURCE_DIR}/calendarsupport
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_SOURCE_DIR}/mailcommon/
  ${CMAKE_SOURCE_DIR}/messagecore/
  ${CMAKE_SOURCE_DIR}/libkdepim/
  ${Boost_INCLUDE_DIRS}
)



set(messageviewer_bodypartformatter_text_calendar_PART_SRCS text_calendar.cpp delegateselector.cpp attendeeselector.cpp memorycalendarmemento.cpp syncitiphandler.cpp)

qt5_add_dbus_interfaces(messageviewer_bodypartformatter_text_calendar_PART_SRCS
  ${CMAKE_SOURCE_DIR}/korganizer/org.kde.Korganizer.Calendar.xml
)

ki18n_wrap_ui(messageviewer_bodypartformatter_text_calendar_PART_SRCS attendeeselector.ui)

add_library(messageviewer_bodypartformatter_text_calendar MODULE ${messageviewer_bodypartformatter_text_calendar_PART_SRCS})


target_link_libraries(messageviewer_bodypartformatter_text_calendar
  KF5::AkonadiContact
  KF5::PimUtils
  Qt5::Core 
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::Mime
  KF5::PimIdentities
  KF5::MailTransport
  KF5::AkonadiMime
  calendarsupport
  kdepim
  mailcommon
  messageviewer
)

target_link_libraries(messageviewer_bodypartformatter_text_calendar incidenceeditorsng)

install(TARGETS messageviewer_bodypartformatter_text_calendar  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES text_calendar.desktop DESTINATION ${DATA_INSTALL_DIR}/messageviewer/plugins/bodypartformatter)
