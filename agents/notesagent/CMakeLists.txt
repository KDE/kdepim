project(notesagent)

include_directories(
   ${CMAKE_SOURCE_DIR}/libkdepim
   ${CMAKE_SOURCE_DIR}/mailcommon
)

set(notesagent_SRCS
    notesagent.cpp
    notesmanager.cpp
    notesagentsettingsdialog.cpp
)

kde4_add_kcfg_files(notesagent_SRCS
    notesagentsettings.kcfgc
)


qt4_add_dbus_adaptor(notesagent_SRCS org.freedesktop.Akonadi.NotesAgent.xml notesagent.h NotesAgent)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

kde4_add_executable(akonadi_notes_agent ${notesagent_SRCS})

target_link_libraries(akonadi_notes_agent
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDE4_KIO_LIBS}
  mailcommon
)

if (Q_WS_MAC)
  set_target_properties(akonadi_notes_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template)
  set_target_properties(akonadi_notes_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.archivemail")
  set_target_properties(akonadi_notes_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Notes")
endif ()

install(TARGETS akonadi_notes_agent ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES notesagent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
install(FILES akonadi_notes_agent.notifyrc DESTINATION "${DATA_INSTALL_DIR}/akonadi_notes_agent" )


