
include_directories(
  ${CMAKE_SOURCE_DIR}/calendarsupport
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_SOURCE_DIR}/mailcommon/
  ${CMAKE_SOURCE_DIR}/messagecore/
  ${CMAKE_SOURCE_DIR}/libkdepim/
  ${Boost_INCLUDE_DIRS}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

########### next target ###############

set(messageviewer_bodypartformatter_text_vcard_PART_SRCS text_vcard.cpp vcardmemento.cpp updatecontactjob.cpp)
kde4_add_plugin(messageviewer_bodypartformatter_text_vcard ${messageviewer_bodypartformatter_text_vcard_PART_SRCS})
target_link_libraries(messageviewer_bodypartformatter_text_vcard ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KMIME_LIBS} ${KDEPIMLIBS_KABC_LIBS} kdepim messageviewer)
install(TARGETS messageviewer_bodypartformatter_text_vcard DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(messageviewer_bodypartformatter_text_calendar_PART_SRCS text_calendar.cpp delegateselector.cpp attendeeselector.cpp memorycalendarmemento.cpp syncitiphandler.cpp)

qt4_add_dbus_interfaces(messageviewer_bodypartformatter_text_calendar_PART_SRCS
  ${CMAKE_SOURCE_DIR}/korganizer/org.kde.Korganizer.Calendar.xml
)

kde4_add_ui_files(messageviewer_bodypartformatter_text_calendar_PART_SRCS attendeeselector.ui)

kde4_add_plugin(messageviewer_bodypartformatter_text_calendar ${messageviewer_bodypartformatter_text_calendar_PART_SRCS})


target_link_libraries(messageviewer_bodypartformatter_text_calendar
  akonadi-calendar
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_KCALUTILS_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  calendarsupport
  kdepim
  mailcommon
  messageviewer
)

target_link_libraries(messageviewer_bodypartformatter_text_calendar incidenceeditorsng)

install(TARGETS messageviewer_bodypartformatter_text_calendar  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(messageviewer_bodypartformatter_text_xdiff_PART_SRCS text_xdiff.cpp)
kde4_add_plugin(messageviewer_bodypartformatter_text_xdiff ${messageviewer_bodypartformatter_text_xdiff_PART_SRCS})
target_link_libraries(messageviewer_bodypartformatter_text_xdiff  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})
install(TARGETS messageviewer_bodypartformatter_text_xdiff  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

kde4_add_plugin(messageviewer_bodypartformatter_application_mstnef application_ms-tnef.cpp)
target_link_libraries(messageviewer_bodypartformatter_application_mstnef
  ${KDE4_KDECORE_LIBS}
  ${QT_QTGUI_LIBRARY}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_KCALUTILS_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KTNEF_LIBS}
  messagecore
  messageviewer
)
install(TARGETS messageviewer_bodypartformatter_application_mstnef DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES text_vcard.desktop text_calendar.desktop text_xdiff.desktop application_ms-tnef.desktop DESTINATION ${DATA_INSTALL_DIR}/messageviewer/plugins/bodypartformatter)
