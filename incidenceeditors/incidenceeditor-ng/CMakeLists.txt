include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(incidenceeditors_ng_shared_LIB_SRCS
  alarmpresets.cpp
  alarmdialog.cpp
  editoritemmanager.cpp # TODO: Move to akonadi libs when finished
  recurrencepresets.cpp

  # Shared incidence editors code
  incidenceeditor.cpp
  combinedincidenceeditor.cpp
  # Specific editors
  incidencealarm.cpp
  incidencegeneral.cpp
  incidencedatetime.cpp
  incidencerecurrence.cpp
  incidencerecurrenceeditor.cpp
  incidencerecurrencedialog.cpp

  ../autochecktreewidget.cpp
  ../categoryconfig.cpp
  ../categoryhierarchyreader.cpp
  ../categoryselectdialog.cpp
  ../editorconfig.cpp
  ../korganizereditorconfig.cpp
)

### Build the desktop version

set( incidenceeditors_ng_desktop_LIB_SRCS
  ${incidenceeditors_ng_shared_LIB_SRCS}

  attachmenticonview.cpp
  attachmenteditdialog.cpp
  incidenceattachment.cpp
  incidencedescription.cpp

  #incidenceeditorgeneralpage.cpp
  #eventortododialog.cpp
  eventortododialogng.cpp
)

kde4_add_ui_files( incidenceeditors_ng_desktop_LIB_SRCS
  eventortododesktop.ui
  alarmdialog.ui
  attachmenteditdialog.ui
  incidencerecurrenceeditor.ui
)

kde4_add_library(incidenceeditorsng SHARED ${incidenceeditors_ng_desktop_LIB_SRCS})

target_link_libraries(incidenceeditorsng
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_KCAL_LIBS}
  ${KDEPIMLIBS_AKONADI_KCAL_LIBRARY}
  kdepim               # For KPIM::K{Date|Time}Edit
  kdepimdbusinterfaces # For UriHandler
  akonadi-kcal_next    # For KCalPrefs
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
    eventortododesktop.ui
    ../editoralarms_base.ui
    incidencerecurrenceeditor.ui
    iegeneralmobile.ui
    iedatetimemobile.ui
  )

  kde4_add_library(incidenceeditorsngmobile SHARED ${incidenceeditors_ng_mobile_LIB_SRCS})

  target_link_libraries(incidenceeditorsngmobile
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDEPIMLIBS_KCAL_LIBS}
    kdepim            # For KPIM::K{Date|Time}Edit
    akonadi-kcal_next # For KCalPrefs
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
  ${KDEPIMLIBS_KCAL_LIBS}
  ${KDEPIMLIBS_AKONADI_KCAL_LIBRARY}
  incidenceeditorsng
  akonadi-kcal_next
  kdepimdbusinterfaces
)

install(TARGETS keventeditor ${INSTALL_TARGETS_DEFAULT_ARGS})
