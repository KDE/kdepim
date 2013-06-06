project( kaddressbook-mobile )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

include_directories(
  ${CMAKE_SOURCE_DIR}/libkleo/
  ${CMAKE_SOURCE_DIR}/libkdepim/
  ${Boost_INCLUDE_DIR}
)

if (WINCE)
  include_directories(
    ${CMAKE_SOURCE_DIR}
  )
endif()

set(kab_mobile_srcs
  mainview.cpp
  main.cpp
  configwidget.cpp
  contactviewitem.cpp
  contactgroupviewitem.cpp
  contactcompletionmodel.cpp
  contacteditorview.cpp
  contactgroupeditorview.cpp
  contactlistproxy.cpp
  contactsexporthandler.cpp
  contactsguistatemanager.cpp
  contactsimporthandler.cpp
  customfieldeditordialog.cpp
  customfieldeditwidget.cpp
  customfieldmanager.cpp
  customfields.cpp
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
  searchwidget.cpp
)

kde4_add_ui_files(kab_mobile_srcs
  editorbusiness.ui
  editorcontactgroup.ui
  editorcrypto.ui
  editorgeneral.ui
  editorlocation.ui
  editormore.ui
  editormore_categoriespage.ui
  editormore_customfieldspage.ui
  editormore_namepage.ui
  editormore_internetpage.ui
  editormore_personalpage.ui
)

qt4_wrap_ui(kab_mobile_srcs configwidget.ui searchwidget.ui)
kde4_add_kcfg_files(kab_mobile_srcs settings.kcfgc)

if (RUNTIME_PLUGINS_STATIC)
  add_definitions(-DMAIL_SERIALIZER_PLUGIN_STATIC)
endif ()

kde4_add_app_icon(kab_mobile_srcs "${CMAKE_CURRENT_SOURCE_DIR}/hi*-apps-kaddressbook-mobile.png")

kde4_add_executable(kaddressbook-mobile ${kab_mobile_srcs} )
if (WINCE)
  SET_TARGET_PROPERTIES(kaddressbook-mobile PROPERTIES LINK_FLAGS " /FIXED:NO")
endif ()

target_link_libraries(kaddressbook-mobile
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDEPIMLIBS_AKONADI_CONTACTS_LIBRARY}
    ${KDEPIMLIBS_KABC_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDE4_KCMUTILS_LIBS}
    kdepim
    kdepimmobileui
    kleo
    incidenceeditorsngmobile
    calendarsupport
)

if (RUNTIME_PLUGINS_STATIC)
  target_link_libraries(kaddressbook-mobile akonadi_serializer_addressee akonadi_serializer_contactgroup)
endif ()
if (WINCE)
  target_link_libraries(kaddressbook-mobile kcm_ldap phone)
endif ()

install(TARGETS kaddressbook-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kaddressbook-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  BulkActionComponent.qml
  ConfigDialog.qml
  contact-editor.qml
  contactgroup-editor.qml
  kaddressbook-mobile.qml
  KAddressBookActions.qml
  kaddressbook-mobile-background.png
  ContactListView.qml
  DESTINATION ${DATA_INSTALL_DIR}/kaddressbook-mobile
)
kde4_install_icons( ${ICON_INSTALL_DIR} )
