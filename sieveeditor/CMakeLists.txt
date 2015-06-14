project( sieveeditor )

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

include_directories(
  ${CMAKE_SOURCE_DIR}/libksieve/ksieveui/
)

set( sieveeditor_kcfg_SRCS settings/sieveeditorglobalconfig.kcfgc )

kconfig_add_kcfg_files(sieveeditor_kcfg_SRCS ${sieveeditor_kcfg_SRCS})


set(sieveeditor_SRCS
  ${sieveeditor_kcfg_SRCS}
  main.cpp
  sieveeditor_debug.cpp
  sieveeditormainwindow.cpp
  serversievesettingsdialog.cpp
  serversievesettings.cpp
  sieveeditormainwidget.cpp
  sieveeditorscriptwidget.cpp
  sieveeditorconfiguredialog.cpp
  sieveeditorscriptmanagerwidget.cpp
  sieveeditorutil.cpp
  serversievelistwidget.cpp
  sieveeditorconfigureserverwidget.cpp
  sieveeditormanagesievewidget.cpp
  sieveeditorpagewidget.cpp
  sieveserversettings.cpp
  sieveeditortabwidget.cpp
  sieveeditorcentralwidget.cpp
  sieveeditorconfigureserverpage.cpp
)

ki18n_wrap_ui(sieveeditor_SRCS
    ui/serversievesettings.ui
    ui/sieveeditorconfigureserverwidget.ui
)


add_executable(sieveeditor ${sieveeditor_SRCS} )
target_link_libraries(sieveeditor
                       KF5::MailTransport
                       KF5::DBusAddons 
                       KF5::I18n
                       pimcommon 
                       ksieveui
                       kmanagesieve
)

install(TARGETS sieveeditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  sieveeditorui.rc
  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/sieveeditor)

install(PROGRAMS org.kde.sieveeditor.desktop DESTINATION ${KDE_INSTALL_APPDIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()
