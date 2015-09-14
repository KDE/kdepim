
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
  kdepimdbusinterfaces
)

set_target_properties(KF5CalendarSupport PROPERTIES
    VERSION ${CALENDARSUPPORT_VERSION_STRING}
    SOVERSION ${CALENDARSUPPORT_SOVERSION}
    EXPORT_NAME CalendarSupport
)

install(TARGETS
    KF5CalendarSupport
    EXPORT KF5CalendarSupportTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
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
  CollectionGeneralPage
  MessageWidget
  ArchiveDialog
  REQUIRED_HEADERS CalendarSupport_HEADERS
  PREFIX CalendarSupport
)

ecm_generate_headers(CalendarSupport_printer_HEADERS
  HEADER_NAMES
  PrintPlugin
  CalPrinter
  CalPrintDefaultPlugins
  REQUIRED_HEADERS CalendarSupport_HEADERS
  PREFIX CalendarSupport
  RELATIVE printing
)

ecm_generate_headers(CalendarSupport_next_HEADERS
  HEADER_NAMES
  IncidenceViewer
  REQUIRED_HEADERS CalendarSupport_HEADERS
  PREFIX CalendarSupport
  RELATIVE next
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${calendarsupport_SOURCE_DIR})
install(FILES
    ${CalendarSupport_CamelCase_HEADERS}
    ${CalendarSupport_printer_HEADERS}
    ${CalendarSupport_next_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/CalendarSupport
    COMPONENT Devel
)

install(FILES
    ${CalendarSupport_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/calendarsupport_export.h
    ${CalendarSupport_CamelCase_HEADERS}
    ${CalendarSupport_printer_HEADERS}
    ${CalendarSupport_next_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/calendarsupport
    COMPONENT Devel
)
endif()





############# Next target #######################

set(calendarsupportcollectionpage_LIB_SRCS
    collectiongeneralpage.cpp
)

add_library(calendarsupportcollectionpage ${calendarsupportcollectionpage_LIB_SRCS})
add_dependencies(calendarsupportcollectionpage KF5::MailCommon)
generate_export_header(calendarsupportcollectionpage BASE_NAME calendarsupportcollectionpage)
target_link_libraries(
  calendarsupportcollectionpage
PUBLIC
  KF5::AkonadiCore
  KF5::MailCommon
PRIVATE
  KF5::CalendarCore
  KF5::PimCommon
  KF5::AkonadiCalendar
)

target_include_directories(calendarsupportcollectionpage PUBLIC "$<BUILD_INTERFACE:${calendarsupport_SOURCE_DIR};${calendarsupport_BINARY_DIR};${Boost_INCLUDE_DIRS}>")

set_target_properties(calendarsupportcollectionpage
  PROPERTIES VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_VERSION}
)

install(TARGETS calendarsupportcollectionpage ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
install(FILES calendarplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})
