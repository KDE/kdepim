remove_definitions(-DKDEPIM_MOBILE_UI)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

#     incidenceeditor-ng/incidenceeditor.cpp
#     incidenceeditor-ng/combinedincidenceeditor.cpp
#     incidenceeditor-ng/incidencegeneraleditor.cpp
#     incidenceeditor-ng/incidencedatetimeeditor.cpp
#     incidenceeditor-ng/incidencedescriptioneditor.cpp
#     incidenceeditor-ng/incidenceattachmenteditor.cpp
#     incidenceeditor-ng/incidencerecurrenceeditor.cpp
#     incidenceeditor-ng/incidencerecurrencedialog.cpp
#     incidenceeditor-ng/incidenceeditorgeneralpage.cpp


   # IncidenceEditorsNG ui files
#    incidenceeditor-ng/attachmenteditdialog.ui
#    incidenceeditor-ng/incidencegeneraleditor.ui
#    incidenceeditor-ng/incidencedatetime.ui
#    incidenceeditor-ng/incidencedescription.ui
#    incidenceeditor-ng/incidenceattachmenteditor.ui
#    incidenceeditor-ng/incidencerecurrenceeditor.ui

set(incidenceeditors_ng_shared_LIB_SRCS
  incidenceeditor.cpp
  combinedincidenceeditor.cpp
  incidencegeneraleditor.cpp
  incidencedatetimeeditor.cpp
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
  incidenceattachmenteditor.cpp
  incidencedescriptioneditor.cpp

  incidenceeditorgeneralpage.cpp
  eventortododialog.cpp
)

kde4_add_ui_files( incidenceeditors_ng_desktop_LIB_SRCS
  attachmenteditdialog.ui
  incidencegeneraleditor.ui
  incidencedatetime.ui
  incidencedescription.ui
  incidenceattachmenteditor.ui
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
    DEFINE_SYMBOL KDEPIM_MOBILE_UI
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
