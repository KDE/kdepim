include_directories(
  ${QT_QTDBUS_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

if (QT_QTDECLARATIVE_FOUND AND KDEPIM_MOBILE_UI)
  add_subdirectory(qml)
endif (QT_QTDECLARATIVE_FOUND AND KDEPIM_MOBILE_UI)

########### next target ###############

set(calendarsupport_LIB_SRCS
  collectionselection.cpp
  calendar.cpp
  calendaradaptor.cpp
  calendarmodel.cpp
  calendarsearch.cpp
  calfilterproxymodel.cpp
  daterangefilterproxymodel.cpp
  incidencefilterproxymodel.cpp
  kcalmodel.cpp
  incidenceviewer.cpp
  utils.cpp
  groupware.cpp
  identitymanager.cpp
  mailclient.cpp
  kcalprefs.cpp
  mailscheduler.cpp
  freebusydownloadjob.cpp
  freebusymanager.cpp
  incidencechanger.cpp
  kcalmodel.cpp
  invitationhandler.cpp
)

if (NOT WINCE)
  set( calendarsupport_LIB_SRCS ${calendarsupport_LIB_SRCS}
    dndfactory.cpp
  )
endif (NOT WINCE)

kde4_add_kcfg_files(calendarsupport_LIB_SRCS kcalprefs_base.kcfgc )

qt4_add_dbus_interface(calendarsupport_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Akonadi.CalendarSearchAgent.xml calendarsearchinterface)

kde4_add_library(calendarsupport ${LIBRARY_TYPE} ${calendarsupport_LIB_SRCS})

target_link_libraries(calendarsupport
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_KCALUTILS_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
)

set_target_properties(calendarsupport PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS calendarsupport ${INSTALL_TARGETS_DEFAULT_ARGS})
