# Commented because it uses your real calendar. You can loose data.
# I'll uncomment this when tests use testrunner.
# add_subdirectory(next/tests)

include_directories(
  ${QT_QTDBUS_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
)
add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)


add_definitions(-DTRANSLATION_DOMAIN=\"calendarsupport\")

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

ki18n_wrap_ui(calendarsupport_LIB_SRCS
  printing/calprintdayconfig_base.ui
  printing/calprintincidenceconfig_base.ui
  printing/calprintjournalconfig_base.ui
  printing/calprintmonthconfig_base.ui
  printing/calprinttodoconfig_base.ui
  printing/calprintweekconfig_base.ui
  printing/calprintyearconfig_base.ui
)

kconfig_add_kcfg_files(calendarsupport_LIB_SRCS kcalprefs_base.kcfgc)

# qt5_add_dbus_interface(calendarsupport_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Akonadi.CalendarSearchAgent.xml calendarsearchinterface)

add_library(calendarsupport ${LIBRARY_TYPE} ${calendarsupport_LIB_SRCS})

target_link_libraries(
  calendarsupport
  kdepimdbusinterfaces
  kdepim
  KF5::AkonadiCore
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::AkonadiContact
  KF5::AkonadiMime
  KF5::Mime
  KF5::PimIdentities
  KF5::PimUtils
  KF5::Holidays
  KF5::MailTransport
  KF5::AkonadiCalendar
  KF5::KIOCore
  #KF5::Ldap
)

set_target_properties(calendarsupport
  PROPERTIES VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS calendarsupport ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES calendarplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
