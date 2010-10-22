add_subdirectory( tests )
#add_subdirectory( next/tests )

include_directories(
  ${QT_QTDBUS_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

########### next target ###############

set(calendarsupport_LIB_SRCS
  akonadicalendar.cpp
  archivedialog.cpp
  attachmenthandler.cpp
  calendar.cpp
  calendaradaptor.cpp
  calendarmodel.cpp
  calendarsearch.cpp
  calendarutils.cpp
  calfilterproxymodel.cpp
  collectionselection.cpp
  daterangefilterproxymodel.cpp
  eventarchiver.cpp
  freebusydownloadjob.cpp
  freebusymanager.cpp
  groupware.cpp
  identitymanager.cpp
  incidencechanger.cpp
  incidencefilterproxymodel.cpp
  incidencesearchjob.cpp
  incidenceviewer.cpp
  invitationhandler.cpp
  kcalmodel.cpp
  kcalprefs.cpp
  mailclient.cpp
  mailscheduler.cpp
  publishdialog.cpp
  utils.cpp

  next/history.cpp
  next/incidencechanger2.cpp
)

if (NOT WINCE)
  set( calendarsupport_LIB_SRCS ${calendarsupport_LIB_SRCS}
    dndfactory.cpp
  )
endif (NOT WINCE)

kde4_add_ui_files(calendarsupport_LIB_SRCS publishdialog_base.ui)

kde4_add_kcfg_files(calendarsupport_LIB_SRCS kcalprefs_base.kcfgc )

qt4_add_dbus_interface(calendarsupport_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Akonadi.CalendarSearchAgent.xml calendarsearchinterface)

kde4_add_library(calendarsupport ${LIBRARY_TYPE} ${calendarsupport_LIB_SRCS})

target_link_libraries(calendarsupport
  akonadi_next
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
)

set_target_properties(calendarsupport PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install( TARGETS calendarsupport ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES calendarplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
