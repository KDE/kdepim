project(kmail-mobile)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

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
  ${SOPRANO_INCLUDE_DIR}
  ${NEPOMUK_CORE_INCLUDE_DIR}
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
  emailsguistatemanager.cpp
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

qt4_wrap_ui(kmail_mobile_srcs configwidget.ui messagelistsettingseditor.ui searchwidget.ui)

if(KDEPIM_ENTERPRISE_BUILD)
  set(WARN_TOOMANY_RECIPIENTS_DEFAULT true)
else()
  set(WARN_TOOMANY_RECIPIENTS_DEFAULT false)
endif()

configure_file(settings.kcfg.cmake ${CMAKE_CURRENT_BINARY_DIR}/settings.kcfg)
kde4_add_kcfg_files(kmail_mobile_srcs settings.kcfgc)

if(RUNTIME_PLUGINS_STATIC)
  add_definitions(-DMAIL_SERIALIZER_PLUGIN_STATIC)
endif()

kde4_add_app_icon(kmail_mobile_srcs "${CMAKE_CURRENT_SOURCE_DIR}/hi*-apps-kmail-mobile.png")

kde4_add_executable(kmail-mobile ${kmail_mobile_srcs})
if(WINCE)
  set_target_properties(kmail-mobile PROPERTIES LINK_FLAGS " /FIXED:NO")
endif()

target_link_libraries(kmail-mobile
  ${QT_QTDECLARATIVE_LIBRARY}
  ${KDE4_KCMUTILS_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${KDEPIMLIBS_KMBOX_LIBS}
  akonadi-calendar
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

if(WINCE)
  target_link_libraries(kmail-mobile
    kcm_kpimidentities
    messageviewer_bodypartformatter_text_calendar
    incidenceeditorsngmobile
  )
endif()

install(TARGETS kmail-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kmail-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  AboutPage.qml
  AccountSettingsPage.qml
  AclEditor.qml
  AttachmentEditor.qml
  ConfigDialog.qml
  FilterEditor.qml
  FolderPage.qml
  ForwardOptionsPage.qml
  HeaderView.qml
  MarkAsPage.qml
  MailListPage.qml
  MailViewPage.qml
  NewMailPage.qml
  ReplyOptionsPage.qml
  EditorView.qml
  FavoriteManagerPage.qml
  kmail-composer.qml
  kmail-mobile.qml
  KMailActions.qml
  KMailComposerActions.qml
  kmail-mobile.notifyrc
  SettingsPage.qml
  SnippetsEditor.qml
  SnippetsList.qml
  StartPage.qml
  VacationScriptIndicator.qml
  add-attachment-button.png
  remove-attachment-button.png
  toggle-signature-button.png
  toggle-encryption-button.png
  DESTINATION ${DATA_INSTALL_DIR}/kmail-mobile
)

kde4_install_icons(${ICON_INSTALL_DIR})

