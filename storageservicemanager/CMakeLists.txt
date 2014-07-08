project( storageservicemanager )

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon/
  ${CMAKE_SOURCE_DIR}/libkdepim/
)

set( storageservicemanager_kcfg_SRCS settings/storageservicemanagerglobalconfig.kcfgc )

kconfig_add_kcfg_files(storageservicemanager_kcfg_SRCS ${storageservicemanager_kcfg_SRCS})

set(storageservicemanager_SRCS
  ${storageservicemanager_kcfg_SRCS}
  main.cpp
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
TARGET_LINK_LIBRARIES(storageservicemanager 
                       KF5::KIOCore
                       KF5::NotifyConfig
                       pimcommon
                       kdepim
)

install(TARGETS storageservicemanager ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  storageservicemanagerui.rc
  DESTINATION  ${KXMLGUI_INSTALL_DIR}/storageservicemanager)

install(PROGRAMS storageservicemanager.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

install(FILES data/storageservicemanager.notifyrc
    DESTINATION ${KNOTIFYRC_INSTALL_DIR}
  )

#READD IT
#add_subdirectory(tests)
