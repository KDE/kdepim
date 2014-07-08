
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} 
                     ${CMAKE_CURRENT_BINARY_DIR}  
                     ${CMAKE_SOURCE_DIR}/libkdepim
                     ${CMAKE_SOURCE_DIR}/pimcommon)

set(blogilo_SRCS
 main.cpp
 mainwindow.cpp
 toolbox.cpp
 addeditblog.cpp
 postentry.cpp
 dbman.cpp
 global.cpp
 bilbopost.cpp
 bilboblog.cpp
 backend.cpp
 entriescountdialog.cpp
 bilbomedia.cpp
 catcheckbox.cpp
 sendtoblogdialog.cpp
 waitwidget.cpp
 blogsettings.cpp
 uploadmediadialog.cpp
 syncuploader.cpp
 poststabwidget.cpp
 configuredialog.cpp
 composer/htmleditor.cpp
 composer/stylegetter.cpp

 composer/blogilocomposerview.cpp
 composer/blogilocomposereditor.cpp
 composer/bilbobrowser.cpp
 composer/blogilocomposerwidget.cpp
 storageservice/storageservicemanagersettingsjob.cpp
 configurestorageservicewidget.cpp
)


ki18n_wrap_ui(
   blogilo_SRCS
  ui/toolboxbase.ui
  ui/addeditblogbase.ui
  ui/entriescountdialogbase.ui
  ui/settingsbase.ui
  ui/editorsettingsbase.ui
  ui/advancedsettingsbase.ui
  ui/sendtoblogbase.ui
  ui/waitwidgetbase.ui
  ui/blogsettingsbase.ui
  ui/uploadmediabase.ui
)

kconfig_add_kcfg_files( blogilo_SRCS settings.kcfgc)
kde4_add_app_icon(blogilo_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-blogilo.png")
add_executable( blogilo ${blogilo_SRCS})

target_link_libraries( blogilo kdepim  KF5::TextEditor  KF5::WebKit KF5::Blog composereditorng  pimcommon KF5::Wallet Qt5::Sql)


install( TARGETS blogilo ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS ../blogilo.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES blogilo.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES blogiloui.rc DESTINATION  ${KXMLGUI_INSTALL_DIR}/blogilo )
