project( sieveeditor )

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

include_directories(
  ${CMAKE_SOURCE_DIR}/libksieve/
  ${CMAKE_SOURCE_DIR}/pimcommon/
  ${CMAKE_SOURCE_DIR}/libksieve/ksieveui/
)

set( sieveeditor_kcfg_SRCS settings/sieveeditorglobalconfig.kcfgc )

kconfig_add_kcfg_files(sieveeditor_kcfg_SRCS ${sieveeditor_kcfg_SRCS})


set(sieveeditor_SRCS
  ${sieveeditor_kcfg_SRCS}
  main.cpp
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
TARGET_LINK_LIBRARIES(sieveeditor 
                       KF5::NewStuff
                       KF5::DBusAddons 
                       pimcommon 
                       ksieveui
                       kmanagesieve
)

install(TARGETS sieveeditor ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  sieveeditorui.rc
  DESTINATION  ${DATA_INSTALL_DIR}/sieveeditor)

install(PROGRAMS sieveeditor.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
