
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


add_definitions(-DTRANSLATION_DOMAIN=\"libpimcommon\")

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
  widgets/annotationdialog.cpp
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

set(libpimcommon_webshortscut_SRCS
   webshortcut/webshortcutsmenumanager.cpp
)

set(libpimcommon_manageserversidesubscription_SRCS
   manageserversidesubscription/manageserversidesubscriptionjob.cpp
)

set(libpimcommon_shorturl_SRCS
   customtools/shorturlplugin/shorturl/shorturlengineplugin/shorturlengineplugin.cpp
)

set(libpimcommon_SRCS
  settings/pimcommonsettings.cpp
  ${libpimcommon_shorturl_SRCS}
  ${libpimcommon_manageserversidesubscription_SRCS}
  ${libpimcommon_webshortscut_SRCS}
  ${libpimcommon_shareserviceurl_SRCS}
  ${libpimcommon_migrate_SRCS}
  ${libpimcommon_util_SRCS}
  ${libpimcommon_texteditor_SRCS}
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
)

macro(add_resource_iface _kcfgFile _ifaceName _className _fileName)
  kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/${_kcfgFile} ${_ifaceName})
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml PROPERTIES INCLUDE "pimcommon/metatype.h")
  qt5_add_dbus_interface(libpimcommon_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_fileName} ${_className}
  )
endmacro()

add_resource_iface(util/imapresource.kcfg org.kde.Akonadi.Imap.Settings ImapSettings imapresourcesettings)

kconfig_add_kcfg_files(libpimcommon_SRCS
  settings/pimcommonsetting_base.kcfgc
)

add_library(KF5PimCommon ${libpimcommon_SRCS})
generate_export_header(KF5PimCommon BASE_NAME pimcommon)
add_library(KF5::PimCommon ALIAS KF5PimCommon)

if( KF5GAPI_FOUND )
  target_link_libraries(KF5PimCommon
    PRIVATE
      KF5::GAPIDrive
  )
endif()

if ( Qt5TextToSpeech_FOUND ) 
  target_link_libraries(KF5PimCommon
    PRIVATE Qt5::TextToSpeech
  )
endif()

target_link_libraries(KF5PimCommon
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
    KF5::Libkdepim
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

target_include_directories(KF5PimCommon INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/PimCommon/;${KDE_INSTALL_INCLUDEDIR_KF5}/pimcommon>")

target_include_directories(KF5PimCommon PUBLIC "$<BUILD_INTERFACE:${pimcommon_SOURCE_DIR}/src;${pimcommon_BINARY_DIR}/src>")
target_include_directories(KF5PimCommon PUBLIC $<TARGET_PROPERTY:KF5::IMAP,INTERFACE_INCLUDE_DIRECTORIES>)

set_target_properties(KF5PimCommon PROPERTIES
    VERSION ${PIMCOMMON_VERSION_STRING}
    SOVERSION ${PIMCOMMON_SOVERSION}
    EXPORT_NAME PimCommon
)

install(TARGETS
    KF5PimCommon
    EXPORT KF5PimCommonTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
)

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


ecm_generate_headers(PimCommon_CamelCase_HEADERS
  HEADER_NAMES
  PimUtil
  NetworkUtil
  EditorUtil
  CreateResource
  VCardUtil
  MetaType
  ImapResourceCapabilitiesManager
  REQUIRED_HEADERS PimCommon_CamelCase_HEADERS
  PREFIX PimCommon
  RELATIVE util
)

ecm_generate_headers(PimCommon_shorturl_HEADERS
  HEADER_NAMES
  ShortUrlEnginePlugin
  REQUIRED_HEADERS PimCommon_shorturl_HEADERS
  PREFIX PimCommon
  RELATIVE customtools/shorturlplugin/shorturl/shorturlengineplugin/
)


ecm_generate_headers(PimCommon_storageservice_HEADERS
  HEADER_NAMES
  StorageServiceAbstract
  StorageServiceManager
  StorageServiceProgressManager
  StorageServiceJobConfig
  REQUIRED_HEADERS PimCommon_storageservice_HEADERS
  PREFIX PimCommon
  RELATIVE storageservice
)

ecm_generate_headers(PimCommon_storageservice_dialog_HEADERS
  HEADER_NAMES
  StorageServiceCheckNameDialog
  REQUIRED_HEADERS PimCommon_storageservice_dialog_HEADERS
  PREFIX PimCommon
  RELATIVE storageservice/dialog/
)


ecm_generate_headers(PimCommon_settings_HEADERS
  HEADER_NAMES
  PimCommonSettings
  REQUIRED_HEADERS PimCommon_settings_HEADERS
  PREFIX PimCommon
  RELATIVE settings
)

ecm_generate_headers(PimCommon_storageservicewidgets_HEADERS
  HEADER_NAMES
  StorageServiceProgressWidget
  StorageServiceTreeWidget
  StorageServiceConfigureWidget
  StorageServiceProgressIndicator
  REQUIRED_HEADERS PimCommon_storageservicewidgets_HEADERS
  PREFIX PimCommon
  RELATIVE storageservice/widgets
)

ecm_generate_headers(PimCommon_storageservicedialog_HEADERS
  HEADER_NAMES
  StorageServiceConfigureDialog
  REQUIRED_HEADERS PimCommon_storageservicedialog_HEADERS
  PREFIX PimCommon
  RELATIVE storageservice/dialog
)

ecm_generate_headers(PimCommon_storageservicesettings_HEADERS
  HEADER_NAMES
  StorageServiceSettings
  StorageServiceSettingsWidget
  REQUIRED_HEADERS PimCommon_storageservicesettings_HEADERS
  PREFIX PimCommon
  RELATIVE storageservice/settings
)

ecm_generate_headers(PimCommon_webshortcut_HEADERS
  HEADER_NAMES
  WebShortCutsMenuManager
  REQUIRED_HEADERS PimCommon_webshortcut_HEADERS
  PREFIX PimCommon
  RELATIVE webshortcut
)

ecm_generate_headers(PimCommon_shareserviceurl_HEADERS
  HEADER_NAMES
  ShareServiceUrlManager
  REQUIRED_HEADERS PimCommon_shareserviceurl_HEADERS
  PREFIX PimCommon
  RELATIVE shareserviceurl
)

ecm_generate_headers(PimCommon_translator_HEADERS
  HEADER_NAMES
  TranslatorWidget
  REQUIRED_HEADERS PimCommon_translator_HEADERS
  PREFIX PimCommon
  RELATIVE translator
)

ecm_generate_headers(PimCommon_customtools_HEADERS
  HEADER_NAMES
  CustomToolsWidgetng
  REQUIRED_HEADERS PimCommon_customtools_HEADERS
  PREFIX PimCommon
  RELATIVE customtools
)

ecm_generate_headers(PimCommon_storageserviceinterface_HEADERS
  HEADER_NAMES
  StorageServiceInterface
  REQUIRED_HEADERS PimCommon_storageserviceinterface_HEADERS
  PREFIX PimCommon
  RELATIVE storageservice/interface
)

ecm_generate_headers(PimCommon_temporaryfile_HEADERS
  HEADER_NAMES
  AttachmentTemporaryFilesDirs
  REQUIRED_HEADERS PimCommon_temporaryfile_HEADERS
  PREFIX PimCommon
  RELATIVE temporaryfile
)

ecm_generate_headers(PimCommon_sievehighlighter_HEADERS
  HEADER_NAMES
  SieveSyntaxHighlighterUtil
  SieveSyntaxHighlighter
  SieveSyntaxHighlighterRules
  REQUIRED_HEADERS PimCommon_sievehighlighter_HEADERS
  PREFIX PimCommon
  RELATIVE sievehighlighter
)


ecm_generate_headers(PimCommon_acl_HEADERS
  HEADER_NAMES
  CollectionAclPage
  ImapAclAttribute
  CollectionAclPage
  REQUIRED_HEADERS PimCommon_acl_HEADERS
  PREFIX PimCommon
  RELATIVE acl
)

ecm_generate_headers(PimCommon_gravatar_HEADERS
  HEADER_NAMES
  GravatarCache
  GravatarResolvUrlJob
  REQUIRED_HEADERS PimCommon_gravatar_HEADERS
  PREFIX PimCommon
  RELATIVE gravatar
)

ecm_generate_headers(PimCommon_migrate_HEADERS
  HEADER_NAMES
  MigrateApplicationFiles
  MigrateFileInfo
  REQUIRED_HEADERS PimCommon_migrate_HEADERS
  PREFIX PimCommon
  RELATIVE migration
)

ecm_generate_headers(PimCommon_autocorrectionwidgets_HEADERS
  HEADER_NAMES
  RichTexteditWithAutoCorrection
  LineEditWithAutoCorrection
  REQUIRED_HEADERS PimCommon_autocorrectionwidgets_HEADERS
  PREFIX PimCommon
  RELATIVE autocorrection/widgets/
)

ecm_generate_headers(PimCommon_managerserversidesubscription_HEADERS
  HEADER_NAMES
  ManageServerSideSubscriptionJob
  REQUIRED_HEADERS PimCommon_managerserversidesubscription_HEADERS
  PREFIX PimCommon
  RELATIVE manageserversidesubscription
)

ecm_generate_headers(PimCommon_autocorrection_HEADERS
  HEADER_NAMES
  AutoCorrectionWidget
  AutoCorrectionLanguage
  AutoCorrection
  REQUIRED_HEADERS PimCommon_autocorrection_HEADERS
  PREFIX PimCommon
  RELATIVE autocorrection/
)

ecm_generate_headers(PimCommon_baloodebug_HEADERS
  HEADER_NAMES
  BalooDebugDialog
  BalooDebugSearchPathComboBox
  REQUIRED_HEADERS PimCommon_baloodebug_HEADERS
  PREFIX PimCommon
  RELATIVE baloodebug
)

ecm_generate_headers(PimCommon_richtexteditor_HEADERS
  HEADER_NAMES
  RichTextEditorWidget
  RichTextEditor
  REQUIRED_HEADERS PimCommon_richtexteditor_HEADERS
  PREFIX PimCommon
  RELATIVE texteditor/richtexteditor
)

ecm_generate_headers(PimCommon_folderdialog_HEADERS
  HEADER_NAMES
  CheckedCollectionWidget
  SelectMultiCollectionDialog
  REQUIRED_HEADERS PimCommon_folderdialog_HEADERS
  PREFIX PimCommon
  RELATIVE folderdialog
)

ecm_generate_headers(PimCommon_texttospeechs_HEADERS
  HEADER_NAMES
  TextToSpeech
  TextToSpeechActions
  TextToSpeechInterface
  TextToSpeechWidget
  AbstractTextToSpeechInterface
  REQUIRED_HEADERS PimCommon_texttospeechs_HEADERS
  PREFIX PimCommon
  RELATIVE texttospeech
)

ecm_generate_headers(PimCommon_templatelist_HEADERS
  HEADER_NAMES
  TemplateListWidget
  TemplateManager
  REQUIRED_HEADERS PimCommon_templatelist_HEADERS
  PREFIX PimCommon
  RELATIVE templatewidgets
)


ecm_generate_headers(PimCommon_plaintexteditor_HEADERS
  HEADER_NAMES
  PlainTextEditor
  PlainTextEditorWidget
  PlainTextEditFindBar
  PlainTextSyntaxSpellCheckingHighlighter
  REQUIRED_HEADERS PimCommon_plaintexteditor_HEADERS
  PREFIX PimCommon
  RELATIVE texteditor/plaintexteditor
)

ecm_generate_headers(PimCommon_texteditor_commonwidget_HEADERS
  HEADER_NAMES
  TextGotoLineWidget
  TextEditFindBarBase 
  REQUIRED_HEADERS PimCommon_texteditor_commonwidget_HEADERS
  PREFIX PimCommon
  RELATIVE texteditor/commonwidget
)

ecm_generate_headers(PimCommon_widgets_HEADERS
  HEADER_NAMES
  CustomTreeView
  SpellCheckLineEdit
  MinimumComboBox
  SlideContainer
  ConfigureImmutableWidgetUtils
  LineEditWithCompleter
  RenameFileDialog
  SimpleStringlistEditor
  KActionMenuChangeCase
  AnnotationDialog
  CustomTextEdit
  REQUIRED_HEADERS PimCommon_widgets_HEADERS
  PREFIX PimCommon
  RELATIVE widgets
)

ecm_generate_pri_file(BASE_NAME PimCommon
    LIB_NAME KF5PimCommon
    DEPS "AkonadiCore TextWidgets AkonadiWidgets Contacts AkonadiContact Network KIOCore ConfigWidgets" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/PimCommon
)


if(${CMAKE_SOURCE_DIR} STREQUAL ${pimcommon_SOURCE_DIR})
install(FILES
    ${PimCommon_storageservicesettings_HEADERS}
    ${PimCommon_storageservice_dialog_HEADERS}
    ${PimCommon_CamelCase_HEADERS}
    ${PimCommon_storageservice_HEADERS}
    ${PimCommon_settings_HEADERS}
    ${PimCommon_storageservicewidgets_HEADERS}
    ${PimCommon_storageservicedialog_HEADERS}
    ${PimCommon_storageservicedialog_HEADERS}
    ${PimCommon_webshortcut_HEADERS}
    ${PimCommon_shareserviceurl_HEADERS}
    ${PimCommon_translator_HEADERS}
    ${PimCommon_customtools_HEADERS}
    ${PimCommon_storageserviceinterface_HEADERS}
    ${PimCommon_temporaryfile_HEADERS}
    ${PimCommon_sievehighlighter_HEADERS}
    ${PimCommon_acl_HEADERS}
    ${PimCommon_gravatar_HEADERS}
    ${PimCommon_migrate_HEADERS}
    ${PimCommon_autocorrectionwidgets_HEADERS}
    ${PimCommon_managerserversidesubscription_HEADERS}
    ${PimCommon_autocorrection_HEADERS}
    ${PimCommon_baloodebug_HEADERS}
    ${PimCommon_richtexteditor_HEADERS}
    ${PimCommon_folderdialog_HEADERS}
    ${PimCommon_texttospeechs_HEADERS}
    ${PimCommon_templatelist_HEADERS}
    ${PimCommon_plaintexteditor_HEADERS}
    ${PimCommon_texteditor_commonwidget_HEADERS}
    ${PimCommon_widgets_HEADERS}

    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/PimCommon
    COMPONENT Devel
)

install(FILES
    ${PimCommon_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/pimcommon_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/pimcommon_debug.h
    ${CMAKE_CURRENT_BINARY_DIR}/pimcommonsetting_base.h
    ${CMAKE_CURRENT_BINARY_DIR}/imapresourcesettings.h
    ${CMAKE_CURRENT_BINARY_DIR}/config-pimcommon.h
    ${PimCommon_storageservicesettings_HEADERS}
    ${PimCommon_storageservice_dialog_HEADERS}
    ${PimCommon_CamelCase_HEADERS}
    ${PimCommon_storageservice_HEADERS}
    ${PimCommon_settings_HEADERS}
    ${PimCommon_storageservicewidgets_HEADERS}
    ${PimCommon_storageservicedialog_HEADERS}
    ${PimCommon_storageservicedialog_HEADERS}
    ${PimCommon_webshortcut_HEADERS}
    ${PimCommon_shareserviceurl_HEADERS}
    ${PimCommon_translator_HEADERS}
    ${PimCommon_customtools_HEADERS}
    ${PimCommon_storageserviceinterface_HEADERS}
    ${PimCommon_temporaryfile_HEADERS}
    ${PimCommon_sievehighlighter_HEADERS}
    ${PimCommon_acl_HEADERS}
    ${PimCommon_gravatar_HEADERS}
    ${PimCommon_migrate_HEADERS}
    ${PimCommon_autocorrectionwidgets_HEADERS}
    ${PimCommon_managerserversidesubscription_HEADERS}
    ${PimCommon_autocorrection_HEADERS}
    ${PimCommon_baloodebug_HEADERS}
    ${PimCommon_richtexteditor_HEADERS}
    ${PimCommon_folderdialog_HEADERS}
    ${PimCommon_texttospeechs_HEADERS}
    ${PimCommon_templatelist_HEADERS}
    ${PimCommon_plaintexteditor_HEADERS}
    ${PimCommon_texteditor_commonwidget_HEADERS}
    ${PimCommon_widgets_HEADERS}

    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/pimcommon
    COMPONENT Devel
)

install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

endif()

add_subdirectory(designer)

add_subdirectory(customtools)
