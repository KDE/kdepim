project( storageservicemanager )

add_definitions(-DTRANSLATION_DOMAIN=\"storageservicemanager\")
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

set( storageservicemanager_kcfg_SRCS settings/storageservicemanagerglobalconfig.kcfgc )

kconfig_add_kcfg_files(storageservicemanager_kcfg_SRCS ${storageservicemanager_kcfg_SRCS})

set(storageservicemanager_SRCS
  ${storageservicemanager_kcfg_SRCS}
  main.cpp
  storageservicemanager_debug.cpp
  storageservicemanagermainwindow.cpp
  storageservicepage.cpp
  storageservicetabwidget.cpp
  storageserviceconfiguredialog.cpp
  storageservicemanagersettingsjob.cpp
  storageservicetreewidget.cpp
  storageservicewarning.cpp
  storageservicelogdialog.cpp
  storageserviceaccountinfodialog.cpp
  storageservicenavigationbuttons.cpp
  storageservicemanagerutil.cpp
  storageservicemanagermainwidget.cpp
  storageserviceconfigurestoragewidget.cpp
)

add_executable(storageservicemanager ${storageservicemanager_SRCS} )
target_link_libraries(storageservicemanager 
                       KF5::NotifyConfig
                       pimcommon
                       kdepim
                       KF5::DBusAddons
)

install(TARGETS storageservicemanager ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  storageservicemanagerui.rc
  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/storageservicemanager)

install(PROGRAMS org.kde.storageservicemanager.desktop DESTINATION ${KDE_INSTALL_APPDIR})

install(FILES data/storageservicemanager.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR})

if (BUILD_TESTING)
   add_subdirectory(autotests)
endif()
