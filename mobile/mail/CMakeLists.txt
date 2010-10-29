project( kmail-mobile )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_subdirectory(tests)

include_directories(
  ${CMAKE_SOURCE_DIR}/libkleo/
  ${CMAKE_SOURCE_DIR}/kmail/
  ${CMAKE_BINARY_DIR}/kmail/kcm_kpimidentities/
  ${CMAKE_SOURCE_DIR}/kmail/kcm_kpimidentities/
  ${CMAKE_SOURCE_DIR}/templateparser
  ${CMAKE_BINARY_DIR}/templateparser
  ${CMAKE_SOURCE_DIR}/messagecomposer
  ${CMAKE_BINARY_DIR}/messagecomposer
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_BINARY_DIR}/messageviewer
  ${Boost_INCLUDE_DIR}
)

set(kmail_mobile_srcs   
  attachmentproxymodel.cpp
  composerautoresizer.cpp
  composerview.cpp
  configwidget.cpp
  declarativeidentitycombobox.cpp
  emailsexporthandler.cpp
  emailsfilterproxymodel.cpp
  emailsimporthandler.cpp
  filterconfigwidget.cpp
  filtermodel.cpp 
  mailactionmanager.cpp
  mainview.cpp
  main.cpp
  messagelistproxy.cpp
  messageviewitem.cpp
  mobilekernel.cpp
  savemailcommand.cpp
  templateemailmodel.cpp
)

qt4_wrap_ui(kmail_mobile_srcs configwidget.ui filterconfigwidget.ui)
kde4_add_kcfg_files(kmail_mobile_srcs settings.kcfgc)

if (RUNTIME_PLUGINS_STATIC)
  add_definitions(-DMAIL_SERIALIZER_PLUGIN_STATIC)
endif (RUNTIME_PLUGINS_STATIC)

kde4_add_executable(kmail-mobile ${kmail_mobile_srcs} )
if (WINCE)
  SET_TARGET_PROPERTIES(kmail-mobile PROPERTIES LINK_FLAGS " /FIXED:NO")
endif (WINCE)

target_link_libraries(kmail-mobile
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDE4_KCMUTILS_LIBS}
    ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
    akonadi_next
    kdepim
    kdepimmobileui
    kmbox
    mailcommon
    messagecomposer
    messagecore
    messageviewer
    templateparser
)

if (RUNTIME_PLUGINS_STATIC)
  target_link_libraries(kmail-mobile akonadi_serializer_mail akonadi_serializer_addressee akonadi_serializer_contactgroup)
endif (RUNTIME_PLUGINS_STATIC)

if (WINCE)
  target_link_libraries(kmail-mobile kcm_kpimidentities messageviewer_bodypartformatter_text_calendar)
endif(WINCE)

install(TARGETS kmail-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kmail-mobile.sh DESTINATION ${BIN_INSTALL_DIR})
install(PROGRAMS kmail-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  ConfigDialog.qml
  FilterActions.qml
  FilterConfigDialog.qml
  HeaderView.qml
  important-email.png
  action-item-email.png
  EditorView.qml
  kmail-composer.qml
  kmail-mobile.qml
  KMailActions.qml
  KMailComposerActions.qml
  kmail-mobile-background.png
  kmail-mobile.notifyrc
  dividing-line.png
  dividing-line-horizontal.png
  DESTINATION ${DATA_INSTALL_DIR}/kmail-mobile
)

kde4_install_icons( ${ICON_INSTALL_DIR} )
