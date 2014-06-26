project(archivemailagent)

include_directories(
   ${CMAKE_SOURCE_DIR}/libkdepim
   ${CMAKE_SOURCE_DIR}/mailcommon
   ${CMAKE_SOURCE_DIR}/messagecomposer/
)

add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_archivemail_agent\")

set(archivemailagent_SRCS
   archivemailkernel.cpp
   archivemailagent.cpp
   archivemailmanager.cpp
   archivemaildialog.cpp
   archivemailinfo.cpp
   addarchivemaildialog.cpp
   archivejob.cpp
   archivemailagentutil.cpp
)

kconfig_add_kcfg_files(archivemailagent_SRCS
    settings/archivemailagentsettings.kcfgc
)


qt5_add_dbus_adaptor(archivemailagent_SRCS org.freedesktop.Akonadi.ArchiveMailAgent.xml archivemailagent.h ArchiveMailAgent)



ki18n_wrap_ui(archivemailagent_SRCS ui/archivemailwidget.ui )

add_executable(akonadi_archivemail_agent ${archivemailagent_SRCS})

target_link_libraries(akonadi_archivemail_agent
  KF5::AkonadiCore
  KF5::PimIdentities
  KF5::Mime
  KF5::AkonadiMime
  KF5::KIOCore
  mailcommon
)

if (Q_WS_MAC)
  set_target_properties(akonadi_archivemail_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template)
  set_target_properties(akonadi_archivemail_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.archivemail")
  set_target_properties(akonadi_archivemail_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Email Archiver")
endif ()

install(TARGETS akonadi_archivemail_agent ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES archivemailagent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
install(FILES akonadi_archivemail_agent.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR} )


