
include_directories(
  ${CMAKE_SOURCE_DIR}/calendarsupport
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_SOURCE_DIR}/mailcommon/
  ${CMAKE_SOURCE_DIR}/messagecore/
  ${CMAKE_SOURCE_DIR}/libkdepim/
  ${Boost_INCLUDE_DIRS}
)

kde_enable_exceptions()

########### next target ###############

set(messageviewer_bodypartformatter_text_vcard_PART_SRCS text_vcard.cpp vcardmemento.cpp updatecontactjob.cpp)
add_library(messageviewer_bodypartformatter_text_vcard MODULE ${messageviewer_bodypartformatter_text_vcard_PART_SRCS})
target_link_libraries(messageviewer_bodypartformatter_text_vcard  KF5::Mime KF5::Abc kdepim messageviewer)
install(TARGETS messageviewer_bodypartformatter_text_vcard DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(messageviewer_bodypartformatter_text_calendar_PART_SRCS text_calendar.cpp delegateselector.cpp attendeeselector.cpp memorycalendarmemento.cpp syncitiphandler.cpp)

qt4_add_dbus_interfaces(messageviewer_bodypartformatter_text_calendar_PART_SRCS
  ${CMAKE_SOURCE_DIR}/korganizer/org.kde.Korganizer.Calendar.xml
)

qt5_wrap_ui(messageviewer_bodypartformatter_text_calendar_PART_SRCS attendeeselector.ui)

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

########### next target ###############

set(messageviewer_bodypartformatter_text_xdiff_PART_SRCS text_xdiff.cpp)
add_library(messageviewer_bodypartformatter_text_xdiff MODULE ${messageviewer_bodypartformatter_text_xdiff_PART_SRCS})
target_link_libraries(messageviewer_bodypartformatter_text_xdiff   )
install(TARGETS messageviewer_bodypartformatter_text_xdiff  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

add_library(messageviewer_bodypartformatter_application_mstnef MODULE application_ms-tnef.cpp)
target_link_libraries(messageviewer_bodypartformatter_application_mstnef
  
  Qt5::Core
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::Mime
  KF5::PimUtils
  KF5::KTnef
  messagecore
  messageviewer
)
install(TARGETS messageviewer_bodypartformatter_application_mstnef DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES text_vcard.desktop text_calendar.desktop text_xdiff.desktop application_ms-tnef.desktop DESTINATION ${DATA_INSTALL_DIR}/messageviewer/plugins/bodypartformatter)
