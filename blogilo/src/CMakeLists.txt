include (KDE4Defaults)
include_directories( ${QT_INCLUDES} ${KDEPIMLIBS_INCLUDE_DIR} ${KDE4_INCLUDES} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(QT_USE_QTSQL 1)

add_subdirectory(htmlconvertors)

IF(WIN32)
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
 addcategory.cpp
 catcheckbox.cpp
 sendtoblogdialog.cpp
 waitwidget.cpp
 blogsettings.cpp
 uploadmediadialog.cpp
 syncuploader.cpp
 composer/bilbobrowser_win.cpp
 composer/htmleditor.cpp
 composer/stylegetter.cpp
 composer/medialistwidget.cpp
 composer/bilboeditor.cpp
 composer/multilinetextedit.cpp
 composer/dialogs/addeditlink.cpp
 composer/dialogs/addmediadialog.cpp
 composer/dialogs/addimagedialog.cpp
)
ELSE(WIN32)
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
 addcategory.cpp
 catcheckbox.cpp
 sendtoblogdialog.cpp
 waitwidget.cpp
 blogsettings.cpp
 uploadmediadialog.cpp
 syncuploader.cpp
 composer/bilbobrowser.cpp
 composer/htmleditor.cpp
 composer/stylegetter.cpp
 composer/medialistwidget.cpp
 composer/bilboeditor.cpp
 composer/multilinetextedit.cpp
 composer/dialogs/addeditlink.cpp
 composer/dialogs/addmediadialog.cpp
 composer/dialogs/addimagedialog.cpp
)
ENDIF(WIN32)

kde4_add_ui_files(
   blogilo_SRCS
  toolboxbase.ui
  addeditblogbase.ui
  entriescountdialogbase.ui
  addcategorybase.ui
  settingsbase.ui
  editorsettingsbase.ui
  advancedsettingsbase.ui
  sendtoblogbase.ui
  waitwidgetbase.ui
  blogsettingsbase.ui
  uploadmediabase.ui
  composer/dialogs/addmediadialogbase.ui
  composer/dialogs/editimagebase.ui
  composer/dialogs/addeditlinkbase.ui
)

kde4_add_kcfg_files( blogilo_SRCS settings.kcfgc)
kde4_add_app_icon(blogilo_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-blogilo.png")
kde4_add_executable( blogilo ${blogilo_SRCS})

IF(WIN32)
target_link_libraries( blogilo ${KDE4_KDEUI_LIBS} ${QT_QTSQL_LIBRARY} ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTWEBKIT_LIBRARY} ${KDE4_KBLOG_LIBRARY} htmlconvertors )
ELSE(WIN32)
target_link_libraries( blogilo ${KDE4_KDEUI_LIBS} ${QT_QTSQL_LIBRARY} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_KHTML_LIBS} ${KDE4_KBLOG_LIBS} htmlconvertors )
ENDIF(WIN32)


install( TARGETS blogilo DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES blogilo.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES blogilo.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES blogiloui.rc  DESTINATION  ${DATA_INSTALL_DIR}/blogilo )
