project( kaddressbook-mobile )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )
add_subdirectory( qmlplugin )

include_directories(
  ${CMAKE_SOURCE_DIR}/akonadi/
  ${Boost_INCLUDE_DIR}
)

set(kab_mobile_srcs
  mainview.cpp
  main.cpp
  contactlistproxy.cpp
  contacteditorview.cpp
  declarativeeditors.cpp
)

kde4_add_executable(kaddressbook-mobile ${kab_mobile_srcs} )

target_link_libraries(kaddressbook-mobile
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDEPIMLIBS_AKONADI_CONTACTS_LIBRARY}
    ${KDEPIMLIBS_KABC_LIBRARY}
    ${KDEPIMLIBS_AKONADI_LIBRARY}
    akonadi_next
    kdepim
    kdepimmobileui
    incidenceeditorsngmobile
)

install(TARGETS kaddressbook-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kaddressbook-mobile.sh DESTINATION ${BIN_INSTALL_DIR})
install(FILES kaddressbook-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  contact-editor.qml
  kaddressbook-mobile.qml
  ContactListView.qml
  DESTINATION ${DATA_INSTALL_DIR}/kaddressbook-mobile
)
kde4_install_icons( ${ICON_INSTALL_DIR} )
