project(pimcommon)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


add_definitions(-DTRANSLATION_DOMAIN=\"libpimcommon\")

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
  shorturl/engine/ur1cashorturl.cpp
  shorturl/engine/isgdshorturl.cpp
)

set(libpimcommon_sieve_SRCS
  sievehighlighter/sievesyntaxhighlighter.cpp
  sievehighlighter/sievesyntaxhighlighterrules.cpp
  sievehighlighter/sievesyntaxhighlighterutil.cpp
)

if( KF5GAPI_FOUND )
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
  storageservice/authdialog/storageauthwebview.cpp

  storageservice/hubic/hubicstorageservice.cpp
  storageservice/hubic/hubicjob.cpp

  storageservice/settings/storageservicecombobox.cpp
  storageservice/settings/addservicestoragedialog.cpp
  storageservice/settings/storagelistwidgetitem.cpp
  storageservice/settings/storageservicesettingswidget.cpp

  storageservice/yousendit/yousenditjob.cpp
  storageservice/yousendit/yousenditstorageservice.cpp
  storageservice/yousendit/yousenditutil.cpp

  storageservice/webdav/webdavstorageservice.cpp
  storageservice/webdav/webdavjob.cpp
  storageservice/webdav/webdavsettingsdialog.cpp
  storageservice/webdav/protocol/webdav_url_info.cpp

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
  storageservice/widgets/storageservicetreewidgetitem.cpp
 
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
  texteditor/plaintexteditor/plaintextsyntaxspellcheckinghighlighter.cpp
  texteditor/commonwidget/textfindreplacewidget.cpp
  texteditor/commonwidget/texteditfindbarbase.cpp
  texteditor/commonwidget/textgotolinewidget.cpp
  texteditor/commonwidget/textmessageindicator.cpp
  texteditor/richtexteditor/richtexteditor.cpp
  texteditor/richtexteditor/richtexteditfindbar.cpp
  texteditor/richtexteditor/richtexteditorwidget.cpp
  texteditor/richtexteditor/richtextdecorator.cpp
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
  acl/collectionaclwidget.cpp
  acl/acllistview.cpp
)


set(libpimcommon_widgets_SRCS
  widgets/minimumcombobox.cpp
  widgets/renamefiledialog.cpp
  widgets/simplestringlisteditor.cpp
  widgets/customtextedit.cpp
  widgets/customtreeview.cpp
  widgets/configureimmutablewidgetutils.cpp
  widgets/slidecontainer.cpp
  widgets/emailaddressrequester.cpp
  widgets/annotationdialog.cpp
  widgets/manageaccountwidget.cpp
  widgets/kactionmenuchangecase.cpp
  widgets/lineeditwithcompleter.cpp
  widgets/spellchecklineedit.cpp
)

set(libpimcommon_customtools_SRCS
  customtools/customtoolsplugin.cpp
  customtools/customtoolspluginmanager.cpp
  customtools/customtoolswidgetng.cpp
  customtools/customtoolsviewinterface.cpp
)

set(libtexttospeech_SRCS
  texttospeech/texttospeech.cpp
  texttospeech/texttospeechwidget.cpp
  texttospeech/texttospeechconfigwidget.cpp
  texttospeech/texttospeechinterface.cpp
  texttospeech/abstracttexttospeechinterface.cpp
  texttospeech/abstracttexttospeechconfiginterface.cpp
  texttospeech/texttospeechconfiginterface.cpp
  texttospeech/texttospeechconfigdialog.cpp
  texttospeech/texttospeechlanguagecombobox.cpp
  texttospeech/texttospeechactions.cpp
)

set(libpimcommon_baloodebug_SRCS
  baloodebug/baloodebugdialog.cpp
  baloodebug/baloodebugwidget.cpp
  baloodebug/job/baloodebugsearchjob.cpp
  baloodebug/baloodebugsearchpathcombobox.cpp
  baloodebug/baloosyntaxhighlighter.cpp
)

set(libpimcommon_util_SRCS
  util/createresource.cpp
  util/pimutil.cpp
  util/editorutil.cpp
  util/imapresourcecapabilitiesmanager.cpp
  util/vcardutil.cpp
  util/networkutil.cpp
)


set(libpimcommon_temporary_SRCS
   temporaryfile/attachmenttemporaryfilesdirs.cpp
)

set(libpimcommon_gravatar_SRCS
   gravatar/gravatarresolvurljob.cpp
   gravatar/gravatarcache.cpp
   gravatar/widgets/gravatardownloadpixmapwidget.cpp
   gravatar/widgets/gravatardownloadpixmapdialog.cpp 
)

set(libpimcommon_migrate_SRCS
   migration/migrateapplicationfiles.cpp  
   migration/migratefileinfo.cpp
)

set(libpimcommon_shareserviceurl_SRCS
   shareserviceurl/shareserviceurlmanager.cpp
)

set(libpimcommon_webshortcut_SRCS
   webshortcut/webshortcutmenumanager.cpp
)

set(libpimcommon_manageserversidesubscription_SRCS
   manageserversidesubscription/manageserversidesubscriptionjob.cpp
)

set(libpimcommon_SRCS
  settings/pimcommonsettings.cpp
  ${libpimcommon_manageserversidesubscription_SRCS}
  ${libpimcommon_webshortcut_SRCS}
  ${libpimcommon_shareserviceurl_SRCS}
  ${libpimcommon_migrate_SRCS}
  ${libpimcommon_util_SRCS}
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
  ${libtexttospeech_SRCS}
  ${libpimcommon_baloodebug_SRCS}
  ${libpimcommon_temporary_SRCS}
  ${libpimcommon_gravatar_SRCS}
  ${libpimcommon_customtools_SRCS}
)

ecm_qt_declare_logging_category(libpimcommon_SRCS HEADER pimcommon_debug.h IDENTIFIER PIMCOMMON_LOG CATEGORY_NAME log_pimcommon)

ki18n_wrap_ui(libpimcommon_SRCS
     autocorrection/ui/autocorrectionwidget.ui
     widgets/ui/manageaccountwidget.ui
)

macro(add_resource_iface _kcfgFile _ifaceName _className _fileName)
  kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/${_kcfgFile} ${_ifaceName})
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml PROPERTIES INCLUDE "util/metatype.h")
  qt5_add_dbus_interface(libpimcommon_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_fileName} ${_className}
  )
endmacro()

add_resource_iface(util/imapresource.kcfg org.kde.Akonadi.Imap.Settings ImapSettings imapresourcesettings)

kconfig_add_kcfg_files(libpimcommon_SRCS
  settings/pimcommonsetting_base.kcfgc
)

add_library(pimcommon ${libpimcommon_SRCS})
generate_export_header(pimcommon BASE_NAME pimcommon)

if( KF5GAPI_FOUND )
  target_link_libraries(pimcommon
    PUBLIC KF5::GAPIDrive
  )
endif()

if ( Qt5TextToSpeech_FOUND ) 
  target_link_libraries(pimcommon
    PUBLIC Qt5::TextToSpeech
  )
endif()

target_link_libraries(pimcommon
PUBLIC
    KF5::AkonadiCore
    KF5::TextWidgets
    KF5::AkonadiWidgets
    KF5::Contacts
    KF5::AkonadiContact
    Qt5::Network
    KF5::KIOCore
    KF5::ConfigWidgets
PRIVATE
    Qt5::WebKitWidgets
    KF5::IMAP
    kdepim
    KF5::Archive
    KF5::Codecs
    KF5::NewStuff
    KF5::PimTextEdit
    KF5::SonnetCore
    KF5::Wallet
    KF5::DBusAddons
    KF5::KDELibs4Support
    KF5::XmlGui
)

target_include_directories(pimcommon PUBLIC "$<BUILD_INTERFACE:${pimcommon_SOURCE_DIR};${pimcommon_BINARY_DIR}>")
target_include_directories(pimcommon PUBLIC "$<BUILD_INTERFACE:${pimcommon_SOURCE_DIR}/shorturl;${pimcommon_BINARY_DIR}/shorturl>")
target_include_directories(pimcommon PUBLIC "$<BUILD_INTERFACE:${pimcommon_SOURCE_DIR}/util;${pimcommon_BINARY_DIR}/util>")
target_include_directories(pimcommon PUBLIC $<TARGET_PROPERTY:KF5::IMAP,INTERFACE_INCLUDE_DIRECTORIES>)

set_target_properties(pimcommon PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS pimcommon ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

if (BUILD_TESTING)
   add_subdirectory(translator/autotests)
   add_subdirectory(texteditor/plaintexteditor/autotests)
   add_subdirectory(texteditor/richtexteditor/autotests)
   add_subdirectory(texteditor/commonwidget/autotests)
   add_subdirectory(autocorrection/autotests)
   add_subdirectory(widgets/autotests)
   add_subdirectory(texttospeech/autotests)
   add_subdirectory(baloodebug/autotests)
   add_subdirectory(baloodebug/tests)
   add_subdirectory(shorturl/autotests)
   add_subdirectory(acl/autotests)
   add_subdirectory(temporaryfile/autotests)
   add_subdirectory(migration/autotests)
   add_subdirectory(util/autotests)
   add_subdirectory(gravatar/autotests)
   add_subdirectory(shareserviceurl/autotests)
   add_subdirectory(webshortcut/autotests)
   add_subdirectory(translator/tests)
   add_subdirectory(texteditor/plaintexteditor/tests)
   add_subdirectory(texteditor/richtexteditor/tests)
   add_subdirectory(widgets/tests)
   add_subdirectory(shorturl/tests)
   add_subdirectory(autocorrection/tests)
   add_subdirectory(folderdialog/tests/)
   add_subdirectory(storageservice/dropbox/tests)
   add_subdirectory(storageservice/hubic/tests)
   add_subdirectory(storageservice/tests)
   add_subdirectory(storageservice/yousendit/tests)
   add_subdirectory(storageservice/webdav/tests)
   add_subdirectory(storageservice/box/tests)
   add_subdirectory(storageservice/dropbox/icons/)
   add_subdirectory(texttospeech/tests)
   add_subdirectory(gravatar/tests)
   add_subdirectory(customtools/autotests)
endif()

if( LibKGAPI2_FOUND )
  add_subdirectory(storageservice/gdrive/tests/)
  add_subdirectory(storageservice/gdrive/icons/)
endif()

if (Qt5Designer_FOUND)
    kf5designerplugin_add_plugin(pimcommonwidgets ${CMAKE_CURRENT_SOURCE_DIR}/designer/pimcommon.widgets)
    target_link_libraries(pimcommonwidgets
        pimcommon
        #KF5::KDELibs4Support
    )
    if(NOT WIN32)
        # Since there are no libraries provided by this module,
        # there is no point including the build tree in RPath,
        # and then having to edit it at install time.
        set_target_properties(pimcommonwidgets PROPERTIES
            SKIP_BUILD_RPATH TRUE
            BUILD_WITH_INSTALL_RPATH TRUE
        )
    endif()

    install(TARGETS
        pimcommonwidgets
        DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/designer
    )
endif()

add_subdirectory(customtools/shorturlplugin)
add_subdirectory(customtools/translatorplugin)
