project( storageservicemanager )

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon/
  ${CMAKE_SOURCE_DIR}/libkdepim/
)

set( storageservicemanager_kcfg_SRCS settings/storageservicemanagerglobalconfig.kcfgc )

kde4_add_kcfg_files(storageservicemanager_kcfg_SRCS ${storageservicemanager_kcfg_SRCS})

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
  storageservicenavigationbar.cpp
)

kde4_add_executable(storageservicemanager ${storageservicemanager_SRCS} )
TARGET_LINK_LIBRARIES(storageservicemanager 
                       ${KDE4_KDEUI_LIBS} 
                       ${KDE4_KIO_LIBS}
		       ${KDE4_SOLID_LIBS}
                       pimcommon
                       kdepim
)

install(TARGETS storageservicemanager ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  storageservicemanagerui.rc
  DESTINATION  ${DATA_INSTALL_DIR}/storageservicemanager)

install(PROGRAMS storageservicemanager.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
