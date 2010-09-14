project( kmail-mobile )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_subdirectory(tests)

include_directories(
  ${CMAKE_SOURCE_DIR}/libkleo/
  ${Boost_INCLUDE_DIR}
)

set(kmail_mobile_srcs
  attachmentproxymodel.cpp
  global.cpp
  composerautoresizer.cpp
  composerview.cpp
  declarativeidentitycombobox.cpp
  mailactionmanager.cpp
  mainview.cpp
  main.cpp
  messagelistproxy.cpp
  messageviewitem.cpp
  savemailcommand.cpp
)
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
    messagecomposer
    messagecore
    messageviewer
    templateparser
)

if (RUNTIME_PLUGINS_STATIC)
  target_link_libraries(kmail-mobile akonadi_serializer_mail)
endif (RUNTIME_PLUGINS_STATIC)

install(TARGETS kmail-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kmail-mobile.sh DESTINATION ${BIN_INSTALL_DIR})
install(FILES kmail-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  HeaderView.qml
  important-email.png
  action-item-email.png
  EditorView.qml
  kmail-composer.qml
  kmail-mobile.qml
  KMailActions.qml
  kmail-mobile-background.png
  kmail-mobile.notifyrc
  DESTINATION ${DATA_INSTALL_DIR}/kmail-mobile
)

kde4_install_icons( ${ICON_INSTALL_DIR} )
