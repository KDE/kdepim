kde4_no_enable_final(calendarsupport)

include_directories(
  ${QT_QTDBUS_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
)
add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

########### next target ###############

set(calendarsupport_LIB_SRCS
  archivedialog.cpp
  attachmenthandler.cpp
  # calendarsearch.cpp
  calendarsingleton.cpp
  calendarutils.cpp
  categoryconfig.cpp
  categoryhierarchyreader.cpp
  cellitem.cpp
  collectiongeneralpage.cpp
  collectionselection.cpp
  eventarchiver.cpp
  identitymanager.cpp
  incidenceattachmentmodel.cpp
  kcalmodel.cpp
  kcalprefs.cpp
  messagewidget.cpp
  utils.cpp

  printing/calprintpluginbase.cpp
  printing/calprintdefaultplugins.cpp
  printing/calprinter.cpp
  printing/journalprint.cpp
  printing/yearprint.cpp

  next/incidenceviewer.cpp
)

kde4_add_ui_files(calendarsupport_LIB_SRCS
  printing/calprintdayconfig_base.ui
  printing/calprintincidenceconfig_base.ui
  printing/calprintjournalconfig_base.ui
  printing/calprintmonthconfig_base.ui
  printing/calprinttodoconfig_base.ui
  printing/calprintweekconfig_base.ui
  printing/calprintyearconfig_base.ui
)

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
  ${KDE4_KUTILS_LIBS}
  akonadi-calendar
)

set_target_properties(calendarsupport
  PROPERTIES VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS calendarsupport ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES calendarplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
