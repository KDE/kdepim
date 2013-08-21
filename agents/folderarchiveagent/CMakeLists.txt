project(folderarchiveagent)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

include_directories(
  ${CMAKE_SOURCE_DIR}/mailcommon
)

set(folderarchivelib_SRCS 
  folderarchiveutil.cpp
)

kde4_add_kcfg_files(folderarchivelib_SRCS
    folderarchiveagentsettings.kcfgc 
  )


kde4_add_library( folderarchive ${LIBRARY_TYPE} ${folderarchivelib_SRCS} )
target_link_libraries( folderarchive ${KDE4_KDEUI_LIBS} )

set_target_properties( folderarchive PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install( TARGETS folderarchive ${INSTALL_TARGETS_DEFAULT_ARGS} )



set(folderarchiveagent_SRCS
  folderarchiveagent.cpp
  folderarchiveconfiguredialog.cpp
  folderarchivesettingpage.cpp
  folderarchivekernel.cpp
  folderarchivemanager.cpp
  folderarchiveaccountinfo.cpp
  folderarchiveagentjob.cpp
  folderarchiveagentcheckcollection.cpp
)

qt4_add_dbus_adaptor(folderarchiveagent_SRCS org.freedesktop.Akonadi.FolderArchiveAgent.xml folderarchiveagent.h FolderArchiveAgent)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

kde4_add_executable(akonadi_folderarchive_agent ${folderarchiveagent_SRCS})

target_link_libraries(akonadi_folderarchive_agent
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  mailcommon
  folderarchive
)

if (Q_WS_MAC)
  set_target_properties(akonadi_folderarchive_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template)
  set_target_properties(akonadi_folderarchive_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.folderarchive")
  set_target_properties(akonadi_folderarchive_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Folder Archive Agent")
endif ()

install(TARGETS akonadi_folderarchive_agent ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES folderarchiveagent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
install(FILES akonadi_folderarchive_agent.notifyrc DESTINATION "${DATA_INSTALL_DIR}/akonadi_folderarchive_agent" )

add_subdirectory(tests)
