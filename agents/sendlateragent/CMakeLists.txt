project(sendlateragent)

add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_sendlater_agent\")

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()

set(sendlater_common_SRCS)
ecm_qt_declare_logging_category(sendlater_common_SRCS HEADER sendlateragent_debug.h IDENTIFIER SENDLATERAGENT_LOG CATEGORY_NAME log_sendlateragent)

set(sendlaterlib_SRCS 
    ${sendlater_common_SRCS}
    sendlaterinfo.cpp
    sendlaterdialog.cpp
    sendlaterutil.cpp
    sendlatertimedatewidget.cpp
)

kconfig_add_kcfg_files(sendlaterlib_SRCS
    settings/sendlateragentsettings.kcfgc 
  )

ki18n_wrap_ui(sendlaterlib_SRCS ui/sendlaterwidget.ui)

add_library( sendlater ${sendlaterlib_SRCS} )
generate_export_header(sendlater BASE_NAME sendlater)
target_link_libraries(sendlater
  PUBLIC
    KF5::AkonadiCore
  PRIVATE
    KF5::ConfigCore
    KF5::I18n
    KF5::WidgetsAddons
    KF5::ConfigGui
    Qt5::DBus
    Qt5::Widgets
    KF5::IconThemes
)
target_include_directories(sendlater PUBLIC "$<BUILD_INTERFACE:${sendlateragent_SOURCE_DIR};${sendlateragent_BINARY_DIR}>")
set_target_properties( sendlater PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION} )

install( TARGETS sendlater ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

############# Next target ###################

set(sendlateragent_SRCS
   ${sendlater_common_SRCS}
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
  KF5::DBusAddons
  KF5::WindowSystem
  KF5::XmlGui
  KF5::Notifications
  KF5::IconThemes
)

if( APPLE )
  set_target_properties(akonadi_sendlater_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template)
  set_target_properties(akonadi_sendlater_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.sendlater")
  set_target_properties(akonadi_sendlater_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Send Later")
endif ()

install(TARGETS akonadi_sendlater_agent ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES sendlateragent.desktop DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents")
install(FILES akonadi_sendlater_agent.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR} )

