project( kaddressbook-mobile )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )
add_subdirectory( qmlplugin )

include_directories(
  ${CMAKE_SOURCE_DIR}/akonadi/
  ${CMAKE_SOURCE_DIR}/libkleo/
  ${Boost_INCLUDE_DIR}
)

set(kab_mobile_srcs
  mainview.cpp
  main.cpp
  contactcompletionmodel.cpp
  contacteditorview.cpp
  contactgroupeditorview.cpp
  contactlistproxy.cpp
  customfieldeditordialog.cpp
  customfieldeditwidget.cpp
  customfieldmanager.cpp
  customfields.cpp
  declarativeeditors.cpp
  displaynameeditwidget.cpp
  editorbase.cpp
  editorbusiness.cpp
  editorcontactgroup.cpp
  editorcrypto.cpp
  editorgeneral.cpp
  editorlocation.cpp
  editormore.cpp
  imagewidget.cpp
  kwidgetlister.cpp
  locationmodel.cpp
  namepartseditwidget.cpp
  mobilelineedit.cpp
  phonetypecombo.cpp
)

kde4_add_ui_files(kab_mobile_srcs
  editorbusiness.ui
  editorcontactgroup.ui
  editorcrypto.ui
  editorgeneral.ui
  editorlocation.ui
  editormore.ui
  editormore_customfieldspage.ui
  editormore_namepage.ui
  editormore_internetpage.ui
  editormore_personalpage.ui
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
    kleo
    incidenceeditorsngmobile
)

configure_file(kaddressbook-mobile.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/kaddressbook-mobile.sh)

install(TARGETS kaddressbook-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/kaddressbook-mobile.sh DESTINATION ${BIN_INSTALL_DIR})
install(FILES kaddressbook-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  contact-editor.qml
  contactgroup-editor.qml
  kaddressbook-mobile.qml
  ContactListView.qml
  DESTINATION ${DATA_INSTALL_DIR}/kaddressbook-mobile
)
kde4_install_icons( ${ICON_INSTALL_DIR} )
