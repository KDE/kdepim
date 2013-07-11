project(folderarchiveagent)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

include_directories(
)

set(folderarchiveagent_SRCS
  folderarchiveagent.cpp
  folderarchiveconfiguredialog.cpp
)

kde4_add_kcfg_files(folderarchiveagent_SRCS
  folderarchiveagentsettings.kcfgc
)


#qt4_add_dbus_adaptor(archivemailagent_SRCS org.freedesktop.Akonadi.ArchiveMailAgent.xml archivemailagent.h ArchiveMailAgent)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

#kde4_add_ui_files(archivemailagent_SRCS ui/archivemailwidget.ui )

kde4_add_executable(akonadi_folderarchive_agent ${folderarchiveagent_SRCS})

target_link_libraries(akonadi_folderarchive_agent
  ${KDEPIMLIBS_AKONADI_LIBS}
)

if (Q_WS_MAC)
  set_target_properties(akonadi_folderarchive_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template)
  set_target_properties(akonadi_folderarchive_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.folderarchive")
  set_target_properties(akonadi_folderarchive_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Folder Archive Agent")
endif ()

install(TARGETS akonadi_folderarchive_agent ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES folderarchiveagent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")


