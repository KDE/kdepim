
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

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
 composer/dialogs/addeditimage.cpp
 composer/texteditor/texteditor.cpp
 composer/texteditor/utilities.cpp
)

else(WIN32)
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
 composer/dialogs/addeditimage.cpp
 composer/texteditor/texteditor.cpp
 composer/texteditor/utilities.cpp
)
endif(WIN32)

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
target_link_libraries( blogilo ${KDE4_KDEUI_LIBS} ${QT_QTSQL_LIBRARY} ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTWEBKIT_LIBRARY} ${KDE4_KDEWEBKIT_LIBRARY} ${KDEPIMLIBS_KBLOG_LIBS} ${QJSON_LIBRARY} htmlconvertors )
ELSE(WIN32)
target_link_libraries( blogilo ${KDE4_KDEUI_LIBS} ${QT_QTSQL_LIBRARY} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_KHTML_LIBS} ${QT_QTWEBKIT_LIBRARY} ${KDE4_KDEWEBKIT_LIBRARY} ${KDEPIMLIBS_KBLOG_LIBS} ${QJSON_LIBRARY} htmlconvertors )
ENDIF(WIN32)


install( TARGETS blogilo ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS ../blogilo.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES blogilo.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES blogiloui.rc composer/texteditor/TextEditorInitialHtml  DESTINATION  ${DATA_INSTALL_DIR}/blogilo )
