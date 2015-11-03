
add_definitions(-DTRANSLATION_DOMAIN=\"libincidenceeditors\")

set(incidenceeditors_ng_shared_LIB_SRCS
  attachmenteditdialog.cpp
  attachmenticonview.cpp
  attendeedata.cpp
  attendeeline.cpp
  attendeecomboboxdelegate.cpp
  attendeelineeditdelegate.cpp
  attendeetablemodel.cpp
  attendeeeditor.cpp

  alarmpresets.cpp
  alarmdialog.cpp
  incidenceeditorsettings.cpp

  ktimezonecombobox.cpp

  # TODO: Move the next two to akonadi libs when finished
  editoritemmanager.cpp

  freebusyurldialog.cpp

  # Shared incidence editors code
  combinedincidenceeditor.cpp
  incidenceeditor.cpp

  # Specific editors
  incidencealarm.cpp
  incidenceattachment.cpp
  incidenceattendee.cpp
  incidencecategories.cpp
  incidencecompletionpriority.cpp
  incidencedescription.cpp
  incidencewhatwhere.cpp
  incidencedatetime.cpp
  incidencerecurrence.cpp
  incidenceresource.cpp
  incidencesecrecy.cpp

  freebusyganttproxymodel.cpp
  conflictresolver.cpp
  schedulingdialog.cpp
  groupwareuidelegate.cpp

  incidencedefaults.cpp
  editorconfig.cpp
  korganizereditorconfig.cpp

  autochecktreewidget.cpp
  categoryeditdialog.cpp
  categoryselectdialog.cpp
  categorydialog.cpp

  # Individual mail
  individualmailcomponentfactory.cpp
  individualmaildialog.cpp
  opencomposerjob.cpp

  # Resourcemanagement
  ldaputils.cpp
  resourcemanagement.cpp
  resourceitem.cpp
  resourcemodel.cpp
)
ecm_qt_declare_logging_category(incidenceeditors_ng_shared_LIB_SRCS HEADER incidenceeditor_debug.h IDENTIFIER INCIDENCEEDITOR_LOG CATEGORY_NAME log_incidenceeditor)

kconfig_add_kcfg_files(incidenceeditors_ng_shared_LIB_SRCS globalsettings_incidenceeditor.kcfgc)

### Build the desktop version

set(incidenceeditors_ng_desktop_LIB_SRCS
  ${incidenceeditors_ng_shared_LIB_SRCS}
  templatemanagementdialog.cpp
  incidencedialogfactory.cpp
  incidencedialog.cpp
  visualfreebusywidget.cpp
)

ki18n_wrap_ui(incidenceeditors_ng_desktop_LIB_SRCS
  ui/categoryeditdialog_base.ui
  ui/categoryselectdialog_base.ui
  ui/categorydialog_base.ui
  ui/template_management_dialog_base.ui
  ui/dialogdesktop.ui
  ui/alarmdialog.ui
  ui/attachmenteditdialog.ui
  ui/schedulingdialog.ui
  ui/resourcemanagement.ui
)

add_library(KF5IncidenceEditorsng ${incidenceeditors_ng_desktop_LIB_SRCS})
generate_export_header(KF5IncidenceEditorsng BASE_NAME incidenceeditors_ng)

add_library(KF5::IncidenceEditorsng ALIAS KF5IncidenceEditorsng)
target_link_libraries(KF5IncidenceEditorsng
PUBLIC
  KF5::AkonadiCalendar
  KF5::AkonadiMime
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::MailTransport
  KF5::CalendarSupport      # For KCalPrefs
  KF5::EventViews
  KF5::KDGantt2             # For FreeBusy Editor
PRIVATE
  KF5::Libkdepim
  KF5::KdepimDBusInterfaces # For UriHandler
  KF5::Codecs
  KF5::Ldap
  KF5::AkonadiWidgets
)
target_include_directories(KF5IncidenceEditorsng INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/IncidenceEditorsng/;${KDE_INSTALL_INCLUDEDIR_KF5}/incidenceeditorsng>")
target_include_directories(KF5IncidenceEditorsng PUBLIC "$<BUILD_INTERFACE:${incidenceeditorsng_SOURCE_DIR}/src;${incidenceeditorsng_BINARY_DIR}/src;>")

set_target_properties(KF5IncidenceEditorsng PROPERTIES
    VERSION ${INCIDENCEEDITORSNG_VERSION_STRING}
    SOVERSION ${INCIDENCEEDITORSNG_SOVERSION}
    EXPORT_NAME IncidenceEditorsng
)

install(TARGETS
    KF5IncidenceEditorsng
    EXPORT KF5IncidenceEditorsngTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS} ${LIBRARY_NAMELINK}
)

ecm_generate_headers(IncidenceEditorsng_CamelCase_HEADERS
  HEADER_NAMES
  IncidenceDialog
  IncidenceDialogFactory
  IncidenceEditorSettings
  CategoryEditDialog
  IncidenceAttendee
  IncidenceDefaults
  IndividualMailComponentFactory
  GroupwareUiDelegate
  EditorItemManager
  OpenComposerJob
  IncidenceEditor-Ng
  REQUIRED_HEADERS IncidenceEditorsng_HEADERS
  PREFIX IncidenceEditorsng
)

ecm_generate_pri_file(BASE_NAME IncidenceEditorsng
    LIB_NAME KF5IncidenceEditorsng
    DEPS "AkonadiCalendar AkonadiMime CalendarCore CalendarUtils MailTransport CalendarSupport EventViews KDGantt2" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/IncidenceEditorsng
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${incidenceeditorsng_SOURCE_DIR})
install(FILES
    ${IncidenceEditorsng_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/IncidenceEditorsng
    COMPONENT Devel
)

install(FILES
    ${IncidenceEditorsng_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/globalsettings_incidenceeditor.h
    ${CMAKE_CURRENT_BINARY_DIR}/incidenceeditors_ng_export.h
    ${IncidenceEditorsng_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/incidenceeditorsng
    COMPONENT Devel
)
install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

endif()



######
if (BUILD_TESTING)
   add_subdirectory(tests)
endif()
