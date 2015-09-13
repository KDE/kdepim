project(incidenceeditorng)

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

add_library(incidenceeditorsng ${incidenceeditors_ng_desktop_LIB_SRCS})
generate_export_header(incidenceeditorsng BASE_NAME incidenceeditors_ng)

target_link_libraries(incidenceeditorsng
PUBLIC
  KF5::AkonadiCalendar
  KF5::AkonadiMime
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::MailTransport
  KF5::CalendarSupport      # For KCalPrefs
  eventviews
  kdgantt2             # For FreeBusy Editor
PRIVATE
  KF5::Libkdepim
  kdepimdbusinterfaces # For UriHandler
  KF5::Codecs
  KF5::Ldap
  KF5::AkonadiWidgets
)

set_target_properties(incidenceeditorsng
  PROPERTIES VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS incidenceeditorsng ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

target_include_directories(incidenceeditorsng PUBLIC "$<BUILD_INTERFACE:${incidenceeditorng_SOURCE_DIR};${incidenceeditorng_BINARY_DIR}>")

set(testincidenceeditor_SRCS main.cpp)

add_executable(kincidenceeditor ${testincidenceeditor_SRCS})

target_link_libraries(kincidenceeditor
  KF5::AkonadiCalendar
  KF5::AkonadiCore
  KF5::CalendarCore
  KF5::CalendarUtils
  incidenceeditorsng
  KF5::CalendarSupport
  kdepimdbusinterfaces
)

install(TARGETS kincidenceeditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

if (BUILD_TESTING)
   add_subdirectory(autotests)
   add_subdirectory(tests)
endif()

