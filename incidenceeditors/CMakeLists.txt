include_directories(
    ${CMAKE_SOURCE_DIR}/incidenceeditors
    ${CMAKE_BINARY_DIR}/incidenceeditors
    ${CMAKE_SOURCE_DIR}/kdgantt1
    ${CMAKE_SOURCE_DIR}/libkdepim
    ${CMAKE_SOURCE_DIR}/libkdepimdbusinterfaces
    ${CMAKE_SOURCE_DIR}/akonadi/kcal
    ${CMAKE_BINARY_DIR}/akonadi/kcal
)

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

set( incidenceeditors_LIB_SRCS
    ${libkdgantt1_SRCS}
    attendeeeditor.cpp
    autochecktreewidget.cpp
    categoryconfig.cpp
    categoryhierarchyreader.cpp
    categoryselectdialog.cpp
    designerfields.cpp
    editoralarms.cpp
    editorattachments.cpp
    editorconfig.cpp
    editordetails.cpp
    editorfreebusy.cpp
    editorgeneral.cpp
    editorgeneralevent.cpp
    editorgeneraljournal.cpp
    editorgeneraltodo.cpp
    editorrecurrence.cpp
    embeddedurlpage.cpp
    eventeditor.cpp
    freebusyurldialog.cpp
    groupwareintegration.cpp
    incidenceeditor.cpp
    journaleditor.cpp
    prefs.cpp
    templatemanagementdialog.cpp
    todoeditor.cpp
)

kde4_add_ui_files( incidenceeditors_LIB_SRCS
   categoryselectdialog_base.ui
   editoralarms_base.ui
   template_management_dialog_base.ui
)

kde4_add_kcfg_files(incidenceeditors_LIB_SRCS prefs_base.kcfgc )

kde4_add_library(incidenceeditors SHARED ${incidenceeditors_LIB_SRCS})

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

target_link_libraries(incidenceeditors
   ${ZLIB_LIBRARY}
   ${QT_QT3SUPPORT_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${QT_QTGUI_LIBRARY}
   ${QT_QTUITOOLS_LIBRARY}
   ${KDE4_KDEUI_LIBS}
   ${KDE4_KPARTS_LIBS}
   ${KDE4_KMIME_LIBRARY}
   ${KDE4_KDE3SUPPORT_LIBRARY}
   ${KDEPIMLIBS_KCAL_LIBS}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KCAL_LIBRARY}
   kdepim
   kdepimdbusinterfaces
   korganizer_core
   akonadi-kcal_next
)

set_target_properties(incidenceeditors PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS incidenceeditors ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory( tests )
