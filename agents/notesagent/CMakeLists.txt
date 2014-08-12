project(notesagent)


include_directories(
   ${CMAKE_SOURCE_DIR}/noteshared
   ${CMAKE_BINARY_DIR}/noteshared
   ${CMAKE_SOURCE_DIR}/pimcommon
)

add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_notes_agent\")

set(notesagent_SRCS
    notesagent.cpp
    notesmanager.cpp
    notesagentsettingsdialog.cpp
    notesagentalarmdialog.cpp
    notesagentnotedialog.cpp
)

kconfig_add_kcfg_files(notesagent_SRCS
    settings/notesagentsettings.kcfgc
)


qt5_add_dbus_adaptor(notesagent_SRCS org.freedesktop.Akonadi.NotesAgent.xml notesagent.h NotesAgent)

add_executable(akonadi_notes_agent ${notesagent_SRCS})

target_link_libraries(akonadi_notes_agent
  KF5::AkonadiCore
  KF5::PimIdentities
  KF5::Mime
  KF5::AkonadiMime
  KF5::KIOCore
  KF5::NotifyConfig
  noteshared
  pimcommon
)

if (Q_WS_MAC)
  set_target_properties(akonadi_notes_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template)
  set_target_properties(akonadi_notes_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.archivemail")
  set_target_properties(akonadi_notes_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Notes")
endif ()

install(TARGETS akonadi_notes_agent ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES notesagent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
install(FILES akonadi_notes_agent.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR} )


add_subdirectory(kconf_update)

