
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  ${CMAKE_SOURCE_DIR}/libkdepim)

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
 composer/htmleditor.cpp
 composer/stylegetter.cpp

 composer/blogilocomposerview.cpp
 composer/blogilocomposereditor.cpp
 composer/bilbobrowser.cpp
)


kde4_add_ui_files(
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

kde4_add_kcfg_files( blogilo_SRCS settings.kcfgc)
kde4_add_app_icon(blogilo_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-blogilo.png")
kde4_add_executable( blogilo ${blogilo_SRCS})

target_link_libraries( blogilo kdepim ${KDE4_KDEUI_LIBS} ${QT_QTSQL_LIBRARY} ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTWEBKIT_LIBRARY} ${KDE4_KDEWEBKIT_LIBRARY} ${KDEPIMLIBS_KBLOG_LIBS} composereditorng )


install( TARGETS blogilo ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS ../blogilo.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES blogilo.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES blogiloui.rc DESTINATION  ${DATA_INSTALL_DIR}/blogilo )
