project( kaddressbook-mobile )



include_directories(
  ${CMAKE_SOURCE_DIR}/libkleo/
  ${CMAKE_SOURCE_DIR}/libkdepim/
  ${Boost_INCLUDE_DIR}
)

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

ki18n_wrap_ui(kab_mobile_srcs
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

ki18n_wrap_ui(kab_mobile_srcs configwidget.ui searchwidget.ui)
kconfig_add_kcfg_files(kab_mobile_srcs settings.kcfgc)

if (RUNTIME_PLUGINS_STATIC)
  add_definitions(-DMAIL_SERIALIZER_PLUGIN_STATIC)
endif ()

kde4_add_app_icon(kab_mobile_srcs "${CMAKE_CURRENT_SOURCE_DIR}/hi*-apps-kaddressbook-mobile.png")

add_executable(kaddressbook-mobile ${kab_mobile_srcs} )

target_link_libraries(kaddressbook-mobile
    Qt5::Declarative
    KF5::Contacts
    KF5::AkonadiCore
    KF5::KCMUtils
    kdepim
    kdepimmobileui
    kleo
    incidenceeditorsngmobile
    calendarsupport
    Phonon::phonon4qt5
)

if (RUNTIME_PLUGINS_STATIC)
  target_link_libraries(kaddressbook-mobile akonadi_serializer_addressee akonadi_serializer_contactgroup)
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
ecm_install_icons(ICONS  hi48-apps-kaddressbook-mobile.png  hi64-apps-kaddressbook-mobile.png  hi80-apps-kaddressbook-mobile-harmattan.png DESTINATION  ${ICON_INSTALL_DIR} )
