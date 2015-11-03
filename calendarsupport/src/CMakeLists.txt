
add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)

add_definitions(-DTRANSLATION_DOMAIN=\"calendarsupport\")

########### next target ###############

set(calendarsupport_LIB_SRCS
  archivedialog.cpp
  attachmenthandler.cpp
  calendarsingleton.cpp
  calendarutils.cpp
  categoryconfig.cpp
  categoryhierarchyreader.cpp
  cellitem.cpp
  collectionselection.cpp
  eventarchiver.cpp
  identitymanager.cpp
  incidenceattachmentmodel.cpp
  kcalmodel.cpp
  kcalprefs.cpp
  messagewidget.cpp
  utils.cpp
  tagcache.cpp

  printing/calprintpluginbase.cpp
  printing/calprintdefaultplugins.cpp
  printing/calprinter.cpp
  printing/journalprint.cpp
  printing/yearprint.cpp

  next/incidenceviewer.cpp

  freebusymodel/freeperiodmodel.cpp
  freebusymodel/freebusyitem.cpp
  freebusymodel/freebusyitemmodel.cpp
  freebusymodel/freebusycalendar.cpp
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
ecm_qt_declare_logging_category(calendarsupport_LIB_SRCS HEADER calendarsupport_debug.h IDENTIFIER CALENDARSUPPORT_LOG CATEGORY_NAME log_calendarsupport)

kconfig_add_kcfg_files(calendarsupport_LIB_SRCS kcalprefs_base.kcfgc)

add_library(KF5CalendarSupport ${calendarsupport_LIB_SRCS})
generate_export_header(KF5CalendarSupport BASE_NAME calendarsupport)
add_library(KF5::CalendarSupport ALIAS KF5CalendarSupport)

target_include_directories(KF5CalendarSupport INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/CalendarSupport/;${KDE_INSTALL_INCLUDEDIR_KF5}/calendarsupport>")
target_include_directories(KF5CalendarSupport PUBLIC "$<BUILD_INTERFACE:${calendarsupport_SOURCE_DIR}/src;${calendarsupport_BINARY_DIR}/src;${Boost_INCLUDE_DIRS}>")

target_link_libraries(
  KF5CalendarSupport
PUBLIC
  KF5::Mime
  KF5::IdentityManagement
  KF5::AkonadiCalendar
PRIVATE
  KF5::AkonadiWidgets
  KF5::Holidays
  KF5::Codecs
  KF5::AkonadiCore
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::KdepimDBusInterfaces
)

set_target_properties(KF5CalendarSupport PROPERTIES
    VERSION ${CALENDARSUPPORT_VERSION_STRING}
    SOVERSION ${CALENDARSUPPORT_SOVERSION}
    EXPORT_NAME CalendarSupport
)

install(TARGETS
    KF5CalendarSupport
    EXPORT KF5CalendarSupportTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS} ${LIBRARY_NAMELINK}
)

ecm_generate_headers(CalendarSupport_CamelCase_HEADERS
  HEADER_NAMES
  Utils
  CategoryConfig
  CellItem
  CollectionSelection
  KCalPrefs
  IdentityManager
  EventArchiver
  Plugin
  CategoryHierarchyReader
  CalendarSingleton
  MessageWidget
  ArchiveDialog
  REQUIRED_HEADERS CalendarSupport_HEADERS
  PREFIX CalendarSupport
)

ecm_generate_headers(CalendarSupport_CamelCaseprinter_HEADERS
  HEADER_NAMES
  PrintPlugin
  CalPrinter
  CalPrintDefaultPlugins
  CalPrintPluginBase
  REQUIRED_HEADERS CalendarSupport_printer_HEADERS
  PREFIX CalendarSupport
  RELATIVE printing
)

ecm_generate_headers(CalendarSupport_CamelCasenext_HEADERS
  HEADER_NAMES
  IncidenceViewer
  REQUIRED_HEADERS CalendarSupport_next_HEADERS
  PREFIX CalendarSupport
  RELATIVE next
)

ecm_generate_headers(CalendarSupport_CamelCasefreebusy_HEADERS
  HEADER_NAMES
  FreeBusyItem
  FreeBusyItemModel
  FreeBusyCalendar
  FreePeriodModel
  REQUIRED_HEADERS CalendarSupport_freebusy_HEADERS
  PREFIX CalendarSupport
  RELATIVE freebusymodel
)

ecm_generate_pri_file(BASE_NAME CalendarSupport
    LIB_NAME KF5CalendarSupport
    DEPS "Mime IdentityManagement AkonadiCalendar" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/CalendarSupport
)



if(${CMAKE_SOURCE_DIR} STREQUAL ${calendarsupport_SOURCE_DIR})
install(FILES
    ${CalendarSupport_CamelCase_HEADERS}
    ${CalendarSupport_CamelCaseprinter_HEADERS}
    ${CalendarSupport_CamelCasenext_HEADERS}
    ${CalendarSupport_CamelCasefreebusy_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/CalendarSupport
    COMPONENT Devel
)

install(FILES
    ${CalendarSupport_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/calendarsupport_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/ui_calprintincidenceconfig_base.h
    ${CMAKE_CURRENT_BINARY_DIR}/ui_calprintdayconfig_base.h
    ${CMAKE_CURRENT_BINARY_DIR}/ui_calprintweekconfig_base.h
    ${CMAKE_CURRENT_BINARY_DIR}/ui_calprintmonthconfig_base.h
    ${CMAKE_CURRENT_BINARY_DIR}/ui_calprinttodoconfig_base.h

    ${CMAKE_CURRENT_BINARY_DIR}/kcalprefs_base.h
    ${CalendarSupport_HEADERS}
    ${CalendarSupport_printer_HEADERS}
    ${CalendarSupport_next_HEADERS}
    ${CalendarSupport_freebusy_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/calendarsupport
    COMPONENT Devel
)
install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

endif()

install(FILES calendarplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})


if(BUILD_TESTING)
   add_subdirectory(freebusymodel/autotests)
endif()


