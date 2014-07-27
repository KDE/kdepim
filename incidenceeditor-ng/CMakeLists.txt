KDE4_NO_ENABLE_FINAL(incidenceeditor-ng)
if(KDEPIM_MOBILE_UI)
  remove_definitions(-DKDEPIM_MOBILE_UI)
endif()

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/calendarsupport
  ${CMAKE_BINARY_DIR}/calendarsupport
  ${CMAKE_SOURCE_DIR}/libkdepim
  ${CMAKE_BINARY_DIR}/incidenceeditor-ng
  ${Boost_INCLUDE_DIR}
)

set(incidenceeditors_ng_shared_LIB_SRCS
  attachmenteditdialog.cpp
  attachmenticonview.cpp
  attendeedata.cpp
  attendeeline.cpp
  attendeeeditor.cpp
  alarmpresets.cpp
  alarmdialog.cpp
  globalsettings.cpp

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

  freebusyitem.cpp
  freebusyitemmodel.cpp
  freebusyganttproxymodel.cpp
  freeperiodmodel.cpp
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
  resourcemanagement.cpp
  resourceitem.cpp
  resourcemodel.cpp
  ldaputils.cpp
  attendeetablemodel.cpp
  attendeecomboboxdelegate.cpp

  # Individual mail
  individualmailcomponentfactory.cpp
  individualmaildialog.cpp
  opencomposerjob.cpp
)

kde4_add_kcfg_files(incidenceeditors_ng_shared_LIB_SRCS globalsettings_base.kcfgc)

### Build the desktop version

set(incidenceeditors_ng_desktop_LIB_SRCS
  ${incidenceeditors_ng_shared_LIB_SRCS}
  templatemanagementdialog.cpp
  incidencedialogfactory.cpp
  incidencedialog.cpp
  visualfreebusywidget.cpp
)

kde4_add_ui_files(incidenceeditors_ng_desktop_LIB_SRCS
  categoryeditdialog_base.ui
  categoryselectdialog_base.ui
  categorydialog_base.ui
  template_management_dialog_base.ui
  dialogdesktop.ui
  alarmdialog.ui
  attachmenteditdialog.ui
  schedulingdialog.ui
  resourcemanagement.ui
)

kde4_add_library(incidenceeditorsng ${LIBRARY_TYPE} ${incidenceeditors_ng_desktop_LIB_SRCS})

target_link_libraries(incidenceeditorsng
  akonadi-calendar
  akonadi-kmime
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QJSON_LIBRARIES}
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_KCALUTILS_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  kdepim
  kdepimdbusinterfaces # For UriHandler
  calendarsupport      # For KCalPrefs
  kdgantt2             # For FreeBusy Editor
)

set_target_properties(incidenceeditorsng
  PROPERTIES VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS incidenceeditorsng ${INSTALL_TARGETS_DEFAULT_ARGS})


### Build the mobile version if needed

#if(KDEPIM_MOBILE_UI)
  set(incidenceeditors_ng_mobile_LIB_SRCS
    ${incidenceeditors_ng_shared_LIB_SRCS}
  )

  kde4_add_ui_files(incidenceeditors_ng_mobile_LIB_SRCS
    categoryeditdialog_base.ui
    categoryselectdialog_base.ui
    categorydialog_base.ui
    editoralarms_base.ui
    dialogdesktop.ui
    dialogmobile.ui
    dialogmoremobile.ui
    attachmenteditdialog.ui
    alarmdialog.ui
    mobileschedulingdialog.ui
  )

  kde4_add_library(incidenceeditorsngmobile ${LIBRARY_TYPE} ${incidenceeditors_ng_mobile_LIB_SRCS})

  target_link_libraries(incidenceeditorsngmobile
    akonadi-calendar
    akonadi-kmime
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    kdepim               # For KPIM::K{Date|Time}Edit
    calendarsupport      # For KCalPrefs
    kdepimdbusinterfaces # For UriHandler
    kdgantt2
    ${KDEPIMLIBS_KCALUTILS_LIBS}
    ${KDEPIMLIBS_KCALCORE_LIBS}
    ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
    ${QJSON_LIBRARIES}
  )

  set_target_properties(incidenceeditorsngmobile PROPERTIES
    COMPILE_FLAGS -DKDEPIM_MOBILE_UI
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
  )

  install(TARGETS incidenceeditorsngmobile ${INSTALL_TARGETS_DEFAULT_ARGS})
#endif(KDEPIM_MOBILE_UI)

### Build the test app TODO: Make optional

set(testincidenceeditor_SRCS main.cpp)

kde4_add_executable(kincidenceeditor ${testincidenceeditor_SRCS})

target_link_libraries(kincidenceeditor
  akonadi-calendar
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_KCALUTILS_LIBS}
  incidenceeditorsng
  calendarsupport
  kdepimdbusinterfaces
)

install(TARGETS kincidenceeditor ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(tests)
