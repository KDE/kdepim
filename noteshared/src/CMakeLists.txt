
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_definitions(-DTRANSLATION_DOMAIN=\"libnoteshared\")

set(libnoteshared_akonadi_SRCS
    akonadi/notesakonaditreemodel.cpp
    akonadi/noteschangerecorder.cpp
)

set(libnoteshared_SRCS
    noteshared_debug.cpp
    attributes/notealarmattribute.cpp
    attributes/notedisplayattribute.cpp
    attributes/notelockattribute.cpp
    attributes/showfoldernotesattribute.cpp
    attributes/attributeregistrar.cpp
    noteutils.cpp
    network/notesnetworksender.cpp
    network/notesnetworkreceiver.cpp
    network/notehostdialog.cpp
    editor/noteeditorutils.cpp
    editor/noteeditor.cpp
    config/noteactionconfig.cpp
    config/notenetworkconfig.cpp
    resources/localresourcecreator.cpp
    job/createnewnotejob.cpp
    dialog/selectednotefolderdialog.cpp
    widget/notelistwidget.cpp
    alarms/notealarmdialog.cpp
    ${libnoteshared_akonadi_SRCS}
)

ecm_qt_declare_logging_category(libnoteshared_SRCS HEADER noteshared_debug.h IDENTIFIER NOTESHARED_LOG CATEGORY_NAME log_noteshared)

macro(add_resource_iface _kcfgFile _ifaceName _className)
  kcfg_generate_dbus_interface(${_kcfgFile} ${_ifaceName})
  string(TOLOWER ${_className} _codeFile)
  set_source_files_properties( ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml PROPERTIES INCLUDE "resources/metatype.h")
  qt5_add_dbus_interface(libnoteshared_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_codeFile} ${_className}
  )
endmacro()

add_resource_iface( ${CMAKE_CURRENT_SOURCE_DIR}/resources/maildirresource.kcfg
                   org.kde.Akonadi.Maildir.Settings MaildirSettings )

add_custom_target(noteshared_settings_xml ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Maildir.Settings.xml
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/maildirresource.kcfg
)



set( libnoteshared_kcfg_lib_SRCS settings/notesharedglobalconfig.kcfgc )

kconfig_add_kcfg_files(libnoteshared_kcfg_lib_SRCS ${libnoteshared_kcfg_lib_SRCS})


add_library(KF5NoteShared ${libnoteshared_SRCS} ${libnoteshared_kcfg_lib_SRCS} )
generate_export_header(KF5NoteShared BASE_NAME noteshared)
add_library(KF5::NoteShared ALIAS KF5NoteShared)

target_include_directories(KF5NoteShared INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/NoteShared/;${KDE_INSTALL_INCLUDEDIR_KF5}/noteshared>")
target_include_directories(KF5NoteShared PUBLIC "$<BUILD_INTERFACE:${noteshared_SOURCE_DIR}/src;${noteshared_BINARY_DIR}/src;>")


target_link_libraries(KF5NoteShared
PUBLIC
     KF5::AkonadiCore
PRIVATE
     KF5::DNSSD KF5::PimCommon  KF5::Mime KF5::AkonadiNotes KF5::KDELibs4Support
)


set_target_properties(KF5NoteShared PROPERTIES
    VERSION ${NOTESHARED_VERSION_STRING}
    SOVERSION ${NOTESHARED_SOVERSION}
    EXPORT_NAME NoteShared
)

install(TARGETS
    KF5NoteShared
    EXPORT KF5NoteSharedTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(NoteShared_CamelCase_HEADERS
  HEADER_NAMES
  NoteUtils
  REQUIRED_HEADERS NoteShared_CamelCase_HEADERS
  PREFIX NoteShared
)

ecm_generate_headers(NoteShared_dialog_HEADERS
  HEADER_NAMES
  SelectedNotefolderDialog
  REQUIRED_HEADERS NoteShared_dialog_HEADERS
  PREFIX NoteShared
  RELATIVE dialog
)



ecm_generate_headers(NoteShared_resources_HEADERS
  HEADER_NAMES
  LocalResourceCreator
  REQUIRED_HEADERS NoteShared_resources_HEADERS
  PREFIX NoteShared
  RELATIVE resources
)


ecm_generate_headers(NoteShared_network_HEADERS
  HEADER_NAMES
  NotesNetworkReceiver
  REQUIRED_HEADERS NoteShared_network_HEADERS
  PREFIX NoteShared
  RELATIVE network/
)


ecm_generate_headers(NoteShared_akonadi_HEADERS
  HEADER_NAMES
  NotesChangeRecorder
  NotesAkonadiTreeModel
  REQUIRED_HEADERS NoteShared_attributes_HEADERS
  PREFIX NoteShared
  RELATIVE akonadi
)


ecm_generate_headers(NoteShared_widget_HEADERS
  HEADER_NAMES
  NoteListWidget
  REQUIRED_HEADERS NoteShared_widget_HEADERS
  PREFIX NoteShared
  RELATIVE widget
)

ecm_generate_headers(NoteShared_attributes_HEADERS
  HEADER_NAMES
  NoteAlarmAttribute
  ShowFolderNotesAttribute
  NoteLockAttribute
  NoteDisplayAttribute
  REQUIRED_HEADERS NoteShared_attributes_HEADERS
  PREFIX NoteShared
  RELATIVE attributes
)

ecm_generate_headers(NoteShared_editor_HEADERS
  HEADER_NAMES
  NoteEditorUtils
  REQUIRED_HEADERS NoteShared_editor_HEADERS
  PREFIX NoteShared
  RELATIVE editor
)

ecm_generate_headers(NoteShared_alarms_HEADERS
  HEADER_NAMES
  NoteAlarmDialog
  REQUIRED_HEADERS NoteShared_alarms_HEADERS
  PREFIX NoteShared
  RELATIVE alarms
)

ecm_generate_headers(NoteShared_config_HEADERS
  HEADER_NAMES
  NoteActionConfig
  NoteNetworkConfig
  REQUIRED_HEADERS NoteShared_config_HEADERS
  PREFIX NoteShared
  RELATIVE config
)

ecm_generate_headers(NoteShared_job_HEADERS
  HEADER_NAMES
  CreateNewNoteJob
  REQUIRED_HEADERS NoteShared_job_HEADERS
  PREFIX NoteShared
  RELATIVE job
)

ecm_generate_pri_file(BASE_NAME NoteShared
    LIB_NAME KF5NoteShared
    DEPS "AkonadiCore" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/NoteShared
)


if(${CMAKE_SOURCE_DIR} STREQUAL ${noteshared_SOURCE_DIR})
install(FILES
    ${NoteShared_CamelCase_HEADERS}
    ${NoteShared_network_HEADERS}
    ${NoteShared_akonadi_HEADERS}
    ${NoteShared_widget_HEADERS}
    ${NoteShared_attributes_HEADERS}
    ${NoteShared_editor_HEADERS}
    ${NoteShared_alarms_HEADERS}
    ${NoteShared_config_HEADERS}
    ${NoteShared_job_HEADERS}
    ${NoteShared_dialog_HEADERS}
    ${NoteShared_resources_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/NoteShared
    COMPONENT Devel
)

install(FILES
    ${NoteShared_dialog_HEADERS}
    ${NoteShared_CamelCase_HEADERS}
    ${NoteShared_network_HEADERS}
    ${NoteShared_akonadi_HEADERS}
    ${NoteShared_widget_HEADERS}
    ${NoteShared_attributes_HEADERS}
    ${NoteShared_editor_HEADERS}
    ${NoteShared_alarms_HEADERS}
    ${NoteShared_config_HEADERS}
    ${NoteShared_job_HEADERS}
    ${NoteShared_resources_HEADERS}

    ${CMAKE_CURRENT_BINARY_DIR}/noteshared_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/notesharedglobalconfig.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/noteshared
    COMPONENT Devel
)
install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

endif()
