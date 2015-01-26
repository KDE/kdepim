project(sendlateragent)


include_directories(
   ${CMAKE_SOURCE_DIR}/pimcommon/
   ${CMAKE_SOURCE_DIR}/messagecomposer/
   ${CMAKE_SOURCE_DIR}/messagecore/
   ${CMAKE_SOURCE_DIR}/libkleo
)


add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_sendlater_agent\")

add_subdirectory(tests)
if (BUILD_TESTING)
   add_subdirectory(autotests)
endif()
set(sendlaterlib_SRCS 
    sendlateragent_debug.cpp
    sendlaterinfo.cpp
    sendlaterdialog.cpp
    sendlaterutil.cpp
    sendlatertimedatewidget.cpp
)

kconfig_add_kcfg_files(sendlaterlib_SRCS
    settings/sendlateragentsettings.kcfgc 
  )

ki18n_wrap_ui(sendlaterlib_SRCS ui/sendlaterwidget.ui)


add_library( sendlater ${LIBRARY_TYPE} ${sendlaterlib_SRCS} )
generate_export_header(sendlater BASE_NAME sendlater)
target_link_libraries( sendlater KF5::AkonadiCore )

set_target_properties( sendlater PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION} )

install( TARGETS sendlater ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )


set(sendlateragent_SRCS
   sendlateragent_debug.cpp
   sendlateragent.cpp
   sendlaterconfiguredialog.cpp
   sendlaterconfigurewidget.cpp
   sendlatermanager.cpp
   sendlaterjob.cpp
   sendlaterremovemessagejob.cpp
)

qt5_add_dbus_adaptor(sendlateragent_SRCS org.freedesktop.Akonadi.SendLaterAgent.xml sendlateragent.h SendLaterAgent)

ki18n_wrap_ui(sendlateragent_SRCS ui/sendlaterconfigurewidget.ui)

add_executable(akonadi_sendlater_agent ${sendlateragent_SRCS})

target_link_libraries(akonadi_sendlater_agent
  sendlater
  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::MailTransport
  KF5::Mime
  messagecomposer
  messagecore
  pimcommon
  KF5::AkonadiAgentBase
)

if( APPLE )
  set_target_properties(akonadi_sendlater_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template)
  set_target_properties(akonadi_sendlater_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.sendlater")
  set_target_properties(akonadi_sendlater_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Send Later")
endif ()

install(TARGETS akonadi_sendlater_agent ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES sendlateragent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
install(FILES akonadi_sendlater_agent.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR} )

