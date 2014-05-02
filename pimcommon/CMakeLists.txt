project(pimcommon)
include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim
  ${CMAKE_CURRENT_SOURCE_DIR}/util/
  ${CMAKE_CURRENT_SOURCE_DIR}/shorturl/
  ${QJSON_INCLUDE_DIR}
)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

kde_enable_exceptions()

set(libpimcommon_shorturl_SRCS
  shorturl/abstractshorturl.cpp
  shorturl/shorturlwidget.cpp
  shorturl/engine/tinyurlshorturl.cpp
  shorturl/engine/googleshorturl.cpp
  shorturl/shorturlconfigurewidget.cpp
  shorturl/shorturlutils.cpp
  shorturl/shorturlconfiguredialog.cpp
  shorturl/engine/migremeshorturl.cpp
  shorturl/engine/triopabshorturl.cpp
)

if( LibKGAPI2_FOUND )
  include_directories(${LibKGAPI2_INCLUDE_DIR})
  set(libpimcommon_storageservice_gdrive 
    storageservice/gdrive/gdrivestorageservice.cpp
    storageservice/gdrive/gdrivejob.cpp
  )
endif()


set(libpimcommon_storageservice_SRCS
  ${libpimcommon_storageservice_gdrive}
  storageservice/storageserviceabstract.cpp
  storageservice/storageservicemanager.cpp
  storageservice/storageserviceprogressmanager.cpp

  storageservice/job/storageserviceabstractjob.cpp

  storageservice/storageservicejobconfig.cpp


  storageservice/dropbox/dropboxstorageservice.cpp
  storageservice/dropbox/dropboxjob.cpp
  storageservice/dropbox/dropboxutil.cpp

  storageservice/authdialog/storageauthviewwidget.cpp
  storageservice/authdialog/storageauthviewdialog.cpp
  storageservice/authdialog/logindialog.cpp

  storageservice/hubic/hubicstorageservice.cpp
  storageservice/hubic/hubicjob.cpp

  storageservice/settings/storageservicecombobox.cpp
  storageservice/settings/addservicestoragedialog.cpp
  storageservice/settings/storagelistwidgetitem.cpp
  storageservice/settings/storageservicesettingswidget.cpp

  storageservice/yousendit/yousenditjob.cpp
  storageservice/yousendit/yousenditstorageservice.cpp
  storageservice/yousendit/yousenditutil.cpp

  #storageservice/webdav/webdavstorageservice.cpp
  #storageservice/webdav/webdavjob.cpp
  #storageservice/webdav/webdavsettingsdialog.cpp
  #storageservice/webdav/protocol/webdav_url_info.cpp

  storageservice/utils/storageserviceutils.cpp

  storageservice/box/boxjob.cpp
  storageservice/box/boxstorageservice.cpp
  storageservice/box/boxutil.cpp

  storageservice/dialog/storageservicedownloaddialog.cpp
  storageservice/dialog/storageservicechecknamedialog.cpp
  storageservice/dialog/storageserviceconfiguredialog.cpp
  storageservice/dialog/storageservicedeletedialog.cpp
  storageservice/dialog/storageservicepropertiesdialog.cpp

  storageservice/widgets/storageserviceconfigurewidget.cpp
  storageservice/widgets/storageservicetreewidget.cpp
  storageservice/widgets/storageserviceprogresswidget.cpp
  storageservice/widgets/storageserviceprogressindicator.cpp
 
  storageservice/settings/storageservicesettings.cpp 
 
  storageservice/tests/testsettingsjob.cpp
)

qt5_add_dbus_adaptor(libpimcommon_storageservice_SRCS storageservice/org.kde.pim.StorageManager.xml storageservice/storageservicemanager.h PimCommon::StorageServiceManager )

set(libpimcommon_autocorrection_SRCS
  autocorrection/autocorrection.cpp
  autocorrection/autocorrectionwidget.cpp
  autocorrection/autocorrectionlistwidget.cpp
  autocorrection/autocorrectiontreewidget.cpp
  autocorrection/autocorrectionlanguage.cpp
  autocorrection/import/importlibreofficeautocorrection.cpp
  autocorrection/import/importkmailautocorrection.cpp
  autocorrection/import/importabstractautocorrection.cpp
  autocorrection/widgets/lineeditwithautocorrection.cpp
  autocorrection/widgets/richtexteditwithautocorrection.cpp
)

set(libpimcommon_texteditor_SRCS
  texteditor/plaintexteditor/plaintexteditfindbar.cpp
  texteditor/plaintexteditor/plaintexteditor.cpp
  texteditor/plaintexteditor/plaintexteditorwidget.cpp
  texteditor/commonwidget/textfindreplacewidget.cpp
  texteditor/commonwidget/texteditfindbarbase.cpp
  texteditor/commonwidget/textgotolinewidget.cpp
  texteditor/richtexteditor/richtexteditor.cpp
  texteditor/richtexteditor/richtexteditfindbar.cpp
  texteditor/richtexteditor/richtexteditorwidget.cpp
)

set(libpimcommon_translator_SRCS
  translator/translatorutil.cpp
  translator/translatorwidget.cpp
  translator/abstracttranslator.cpp
  translator/translatordebugdialog.cpp
  #Laurent: disable it we can't use it directly now
  #translator/babelfishtranslator.cpp
  translator/googletranslator.cpp
)

set(libpimcommon_templatewidgets_SRCS
  templatewidgets/templatelistwidget.cpp
  templatewidgets/templateeditdialog.cpp
  templatewidgets/templatemanager.cpp
)

set(libpimcommon_folderdialog_SRCS
  folderdialog/selectmulticollectionwidget.cpp
  folderdialog/selectmulticollectiondialog.cpp
  folderdialog/checkedcollectionwidget.cpp
)

set(libpimcommon_acl_SRCS
  acl/aclentrydialog.cpp
  acl/aclmanager.cpp
  acl/aclutils.cpp
  acl/collectionaclpage.cpp
  acl/imapaclattribute.cpp
)

set(libpimcommon_sieve_SRCS
  sievehighlighter/sievesyntaxhighlighter.cpp
  sievehighlighter/sievesyntaxhighlighterutil.cpp
)

set(libpimcommon_widgets_SRCS
  widgets/minimumcombobox.cpp
  widgets/renamefiledialog.cpp
  widgets/simplestringlisteditor.cpp
  widgets/customtextedit.cpp
  widgets/customtreeview.cpp
  widgets/customtoolswidget.cpp
  widgets/configureimmutablewidgetutils.cpp
)



set(libpimcommon_SRCS
  util/createresource.cpp
  util/pimutil.cpp
  util/editorutil.cpp
  settings/pimcommonsettings.cpp
  ${libpimcommon_texteditor_SRCS}
  ${libpimcommon_shorturl_SRCS}
  ${libpimcommon_storageservice_SRCS}
  ${libpimcommon_autocorrection_SRCS}
  ${libpimcommon_translator_SRCS}
  ${libpimcommon_templatewidgets_SRCS}
  ${libpimcommon_folderdialog_SRCS}
  ${libpimcommon_acl_SRCS}
  ${libpimcommon_sieve_SRCS}
  ${libpimcommon_widgets_SRCS}
)

qt5_wrap_ui(libpimcommon_SRCS
     autocorrection/ui/autocorrectionwidget.ui
)

macro(add_resource_iface _kcfgFile _ifaceName _className _fileName)
  kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/${_kcfgFile} ${_ifaceName})
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml PROPERTIES INCLUDE "metatype.h")
  qt5_add_dbus_interface(libpimcommon_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_fileName} ${_className}
  )
endmacro()

add_resource_iface(util/imapresource.kcfg org.kde.Akonadi.Imap.Settings ImapSettings imapresourcesettings)

kconfig_add_kcfg_files(libpimcommon_SRCS
  settings/pimcommonsetting_base.kcfgc
)

add_library(pimcommon ${LIBRARY_TYPE} ${libpimcommon_SRCS})

if( LibKGAPI2_FOUND )
  target_link_libraries(pimcommon
    ${LibKGAPI2_LIBRARY}
  )
endif()


target_link_libraries(pimcommon
    KF5::Abc
    KF5::AkonadiContact
    KF5::AkonadiCore
    KF5::PimUtils
    KF5::SonnetCore
    ${QT_QTWEBKIT_LIBRARY}
    ${KDE4_SOLID_LIBS}
    ${LibKGAPI2_LIBRARY}
    KF5::NewStuff
    KF5::Wallet
    kdepim
    #KF5::Ldap
    KF5::Imap
    KF5::AkonadiWidgets
    KF5::Archive
)

set_target_properties(pimcommon PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS pimcommon ${INSTALL_TARGETS_DEFAULT_ARGS})

#add_subdirectory(translator/tests)
#add_subdirectory(texteditor/plaintexteditor/tests)
#add_subdirectory(texteditor/richtexteditor/tests)
#add_subdirectory(texteditor/commonwidget/tests/)
#add_subdirectory(widgets/tests)
#add_subdirectory(shorturl/tests)
#add_subdirectory(autocorrection/tests)
#add_subdirectory(folderdialog/tests/)
#add_subdirectory(storageservice/dropbox/tests)
#add_subdirectory(storageservice/hubic/tests)
#add_subdirectory(storageservice/tests)
#add_subdirectory(storageservice/yousendit/tests)
#add_subdirectory(storageservice/webdav/tests)
#add_subdirectory(storageservice/box/tests)
add_subdirectory(storageservice/dropbox/icons/)

if( LibKGAPI2_FOUND )
add_subdirectory(storageservice/gdrive/tests/)
add_subdirectory(storageservice/gdrive/icons/)
endif()

if (QT_QTDESIGNER_FOUND)
  set(pimcommonwidgets_PART_SRCS)
  kde4_add_widget_files(pimcommonwidgets_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/designer/pimcommon.widgets)

  add_library(pimcommonwidgets MODULE ${pimcommonwidgets_PART_SRCS})


  target_link_libraries(pimcommonwidgets  KF5::KIOCore pimcommon)

  install(TARGETS pimcommonwidgets  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer)
endif ()


