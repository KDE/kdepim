
if (KF5GAPI_FOUND)
  add_definitions(-DHAVE_GAPIBLOGGER_SUPPORT)
endif()

set(blogilo_SRCS
 blogilomigrateapplication.cpp
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

if (KF5GAPI_FOUND)
 # Blogger API implementation imported from KBlog due to circular deps between
 # kdepimlibs and libkgapi
 # TODO: Move back to KBlog in KF5
   set(blogilo_SRCS ${blogilo_SRCS} blogger.cpp)
endif()

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
ecm_qt_declare_logging_category(blogilo_SRCS HEADER blogilo_debug.h IDENTIFIER BLOGILO_LOG CATEGORY_NAME log_blogilo)

kconfig_add_kcfg_files( blogilo_SRCS settings.kcfgc)
file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*-apps-blogilo.png")
ecm_add_app_icon(blogilo_SRCS ICONS ${ICONS_SRCS})
add_executable( blogilo ${blogilo_SRCS})

target_link_libraries(blogilo
    KF5::Blog
    KF5::PimCommon
    Qt5::Sql
    KF5::ComposerEditorNG
    KF5::Libkdepim
    KF5::TextEditor
    KF5::WebKit
    KF5::Wallet
    KF5::DBusAddons
    KF5::XmlGui
)

if (KF5GAPI_FOUND)
   target_link_libraries( blogilo KF5::GAPIBlogger KF5::GAPICore)
endif()


install( TARGETS blogilo ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS ../org.kde.blogilo.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( FILES ../blogilo.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install( FILES blogilo.kcfg  DESTINATION  ${KDE_INSTALL_KCFGDIR} )
install( FILES blogiloui.rc DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/blogilo )
