include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

if(NOT MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")
endif(NOT MSVC)

set(incidenceeditors_ng_shared_LIB_SRCS
  attachmenteditdialog.cpp
  attachmenticonview.cpp
  attendeedata.cpp
  attendeeline.cpp
  attendeeeditor.cpp
  alarmpresets.cpp
  alarmdialog.cpp

  # TODO: Move the next two to akonadi libs when finished
  editoritemmanager.cpp
  invitationdispatcher.cpp

  freebusyurldialog.cpp
  editorfreebusy.cpp

  # Shared incidence editors code
  incidenceeditor.cpp
  combinedincidenceeditor.cpp

  # Specific editors
  incidencealarm.cpp
  incidenceattachment.cpp
  incidenceattendee.cpp
  incidencecategories.cpp
  incidencecompletionpriority.cpp
  incidencedescription.cpp
  incidencegeneral.cpp
  incidencedatetime.cpp
  incidencerecurrence.cpp
  incidencesecrecy.cpp

  freebusyitem.cpp
  freebusyitemmodel.cpp
  freeperiodmodel.cpp
  conflictresolver.cpp

  ../autochecktreewidget.cpp
  ../categoryconfig.cpp
  ../categoryhierarchyreader.cpp
  ../categoryselectdialog.cpp
  ../editorconfig.cpp
  ../korganizereditorconfig.cpp
  ../ktimezonecombobox.cpp
)

### Build the desktop version

set( incidenceeditors_ng_desktop_LIB_SRCS
  ${incidenceeditors_ng_shared_LIB_SRCS}
  ../templatemanagementdialog.cpp
  eventortododialog.cpp
  schedulingdialog.cpp
  visualfreebusywidget.cpp
)

kde4_add_ui_files( incidenceeditors_ng_desktop_LIB_SRCS
  ../template_management_dialog_base.ui
  eventortododesktop.ui
  alarmdialog.ui
  attachmenteditdialog.ui
  schedulingdialog.ui
)

kde4_add_library(incidenceeditorsng ${LIBRARY_TYPE} ${incidenceeditors_ng_desktop_LIB_SRCS})

target_link_libraries(incidenceeditorsng
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_AKONADI_KCAL_LIBRARY}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_KCALUTILS_LIBS}
  kdepim               # For KPIM::K{Date|Time}Edit
  kdepimdbusinterfaces # For UriHandler
  akonadi-kcal_next    # For KCalPrefs
  kdgantt2             # For FreeBusy Editor
)

set_target_properties(incidenceeditorsng
  PROPERTIES VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS incidenceeditorsng ${INSTALL_TARGETS_DEFAULT_ARGS})

### Build the mobile version if needed

if (KDEPIM_MOBILE_UI)
  set (incidenceeditors_ng_mobile_LIB_SRCS
    ${incidenceeditors_ng_shared_LIB_SRCS}
  )

  kde4_add_ui_files(incidenceeditors_ng_mobile_LIB_SRCS
    ../categoryselectdialog_base.ui
    ../editoralarms_base.ui
    eventortododesktop.ui
    eventortodomobile.ui
    eventortodomoremobile.ui
    attachmenteditdialog.ui
    alarmdialog.ui
  )

  kde4_add_library(incidenceeditorsngmobile ${LIBRARY_TYPE} ${incidenceeditors_ng_mobile_LIB_SRCS})

  target_link_libraries(incidenceeditorsngmobile
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDEPIMLIBS_AKONADI_KCAL_LIBRARY}
    kdepim            # For KPIM::K{Date|Time}Edit
    akonadi-kcal_next # For KCalPrefs
    kdepimdbusinterfaces # For UriHandler
    kdgantt2
    ${KDEPIMLIBS_KCALUTILS_LIBS}
    ${KDEPIMLIBS_KCALCORE_LIBS}
  )

  set_target_properties(incidenceeditorsngmobile PROPERTIES
    COMPILE_FLAGS -DKDEPIM_MOBILE_UI
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
  )

  install(TARGETS incidenceeditorsngmobile ${INSTALL_TARGETS_DEFAULT_ARGS})
endif (KDEPIM_MOBILE_UI)


### Build the test app TODO: Make optional

set(testincidenceeditor_SRCS main.cpp)

kde4_add_executable(keventeditor ${testincidenceeditor_SRCS})
target_link_libraries(keventeditor
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_KCALUTILS_LIBS}
  ${KDEPIMLIBS_AKONADI_KCAL_LIBRARY}
  incidenceeditorsng
  akonadi-kcal_next
  kdepimdbusinterfaces
)

install(TARGETS keventeditor ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory( tests )
