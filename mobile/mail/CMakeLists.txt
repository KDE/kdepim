project(kmail-mobile)



add_subdirectory(docs)
add_subdirectory(tests)

include_directories(
  ${CMAKE_SOURCE_DIR}/libkleo/
  ${CMAKE_SOURCE_DIR}/libksieve/
  ${CMAKE_SOURCE_DIR}/libksieve/ksieveui/
  ${CMAKE_SOURCE_DIR}/kmail/
  ${CMAKE_BINARY_DIR}/kmail/kcm_kpimidentities/
  ${CMAKE_SOURCE_DIR}/kmail/kcm_kpimidentities/
  ${CMAKE_SOURCE_DIR}/templateparser
  ${CMAKE_BINARY_DIR}/templateparser
  ${CMAKE_SOURCE_DIR}/messagecore
  ${CMAKE_SOURCE_DIR}/messagecomposer
  ${CMAKE_BINARY_DIR}/messagecomposer
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_BINARY_DIR}/messageviewer
  ${CMAKE_SOURCE_DIR}/mailcommon
  ${CMAKE_SOURCE_DIR}/pimcommon/
  ${CMAKE_SOURCE_DIR}/libkdepim/
  ${Boost_INCLUDE_DIR}
)

set(kmail_mobile_srcs
  acleditor.cpp
  attachmenteditor.cpp
  attachmentproxymodel.cpp
  charsetselectiondialog.cpp
  composerautoresizer.cpp
  composerview.cpp
  configwidget.cpp
  cryptoformatselectiondialog.cpp
  declarativeidentitycombobox.cpp
  emailsexporthandler.cpp
  emailsfilterproxymodel.cpp
  emailsimporthandler.cpp
  filtereditor.cpp
  mailactionmanager.cpp
  mailthreadgroupercomparator.cpp
  mainview.cpp
  main.cpp
  messagelistproxy.cpp
  messagelistsettings.cpp
  messagelistsettingscontroller.cpp
  messageviewitem.cpp
  mobilekernel.cpp
  savemailcommand.cpp
  searchwidget.cpp
  snippetseditor.cpp
  templateemailmodel.cpp
  vacationmanager.cpp
)

ki18n_wrap_ui(kmail_mobile_srcs configwidget.ui messagelistsettingseditor.ui searchwidget.ui)

if(KDEPIM_ENTERPRISE_BUILD)
  set(WARN_TOOMANY_RECIPIENTS_DEFAULT true)
else()
  set(WARN_TOOMANY_RECIPIENTS_DEFAULT false)
endif()

configure_file(settings.kcfg.cmake ${CMAKE_CURRENT_BINARY_DIR}/settings.kcfg)
kconfig_add_kcfg_files(kmail_mobile_srcs settings.kcfgc)

if(RUNTIME_PLUGINS_STATIC)
  add_definitions(-DMAIL_SERIALIZER_PLUGIN_STATIC)
endif()

kde4_add_app_icon(kmail_mobile_srcs "${CMAKE_CURRENT_SOURCE_DIR}/hi*-apps-kmail-mobile.png")

add_executable(kmail-mobile ${kmail_mobile_srcs})

target_link_libraries(kmail-mobile
  Qt5::Declarative
  KF5::KCMUtils
  KF5::AkonadiMime
  KF5::Mime
  KF5::AkonadiCore
  KF5::MailTransport
  KF5::Mbox
  KF5::AkonadiCalendar
  akonadi_next
  kdepim
  kdepimmobileui
  kdepimmobileuicalendar
  ksieveui
  kleo
  mailcommon
  pimcommon
  messagecomposer
  messagecore
  messageviewer
  templateparser
)

if(RUNTIME_PLUGINS_STATIC)
  target_link_libraries(kmail-mobile
    akonadi_serializer_mail
    akonadi_serializer_addressee
    akonadi_serializer_contactgroup
    akonadi_serializer_kcalcore
  )
endif()

install(TARGETS kmail-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kmail-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  AboutPage.qml
  AccountSettingsPage.qml
  AccountsPage.qml
  AclEditor.qml
  AttachmentEditor.qml
  ConfigDialog.qml
  FilterEditor.qml
  FolderPage.qml
  MailListPage.qml
  MailViewPage.qml
  EditorView.qml
  FavoriteManagerPage.qml
  kmail-composer.qml
  kmail-mobile.qml
  KMailComposerActions.qml
  kmail-mobile.notifyrc
  SettingsPage.qml
  SnippetsEditor.qml
  SnippetsList.qml
  add-attachment-button.png
  dummy-avatar.png
  remove-attachment-button.png
  toggle-signature-button.png
  toggle-encryption-button.png
  DESTINATION ${DATA_INSTALL_DIR}/kmail-mobile
)

ecm_install_icons(${ICON_INSTALL_DIR})

