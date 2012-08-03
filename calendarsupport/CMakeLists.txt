# Commented because it uses your real calendar. You can loose data.
# I'll uncomment this when tests use testrunner.
# add_subdirectory(next/tests)

include_directories(
  ${QT_QTDBUS_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

########### next target ###############

set(calendarsupport_LIB_SRCS
  archivedialog.cpp
  attachmenthandler.cpp
  attributeregistrar.cpp
  blockalarmsattribute.cpp
  calendar.cpp
  calendaradaptor.cpp
  calendarmodel.cpp
  # calendarsearch.cpp
  calendarutils.cpp
  calfilterproxymodel.cpp
  categoryconfig.cpp
  collectiongeneralpage.cpp
  collectionselection.cpp
  daterangefilterproxymodel.cpp
  eventarchiver.cpp
  groupware.cpp
  identitymanager.cpp
  incidenceattachmentmodel.cpp
  incidencechanger.cpp
  incidencefilterproxymodel.cpp
  kcalmodel.cpp
  kcalprefs.cpp
  mailclient.cpp
  mailscheduler.cpp
  nepomukcalendar.cpp
  utils.cpp

  next/akonadicalendar.cpp
  next/history.cpp
  next/incidencechanger2.cpp
  next/incidencefetchjob.cpp
  next/incidencesearchjob.cpp
  next/incidenceviewer.cpp
  next/invitationhandler.cpp
  next/scheduler.cpp
)

if(NOT WINCE)
  add_subdirectory(tests)
  set(calendarsupport_LIB_SRCS
    ${calendarsupport_LIB_SRCS}
    dndfactory.cpp
  )
endif()

kde4_add_kcfg_files(calendarsupport_LIB_SRCS kcalprefs_base.kcfgc)

# qt4_add_dbus_interface(calendarsupport_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Akonadi.CalendarSearchAgent.xml calendarsearchinterface)

kde4_add_library(calendarsupport ${LIBRARY_TYPE} ${calendarsupport_LIB_SRCS})

target_link_libraries(
  calendarsupport
  kdepimdbusinterfaces
  kdepim
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_KCALUTILS_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KHOLIDAYS_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  akonadi-calendar
)

set_target_properties(calendarsupport PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS calendarsupport ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES calendarplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
