project(kaddressbook)
kde_enable_exceptions()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/xxport/shared
  ${CMAKE_CURRENT_SOURCE_DIR}/printing/
  ${Boost_INCLUDE_DIR}
  ${Grantlee_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/pimcommon/
  ${CMAKE_SOURCE_DIR}/libkdepim/
  ${CMAKE_SOURCE_DIR}/grantleetheme/
  ${CMAKE_SOURCE_DIR}/kaddressbookgrantlee/
  ${CMAKE_SOURCE_DIR}/libkleo
)


add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


if(Prison_FOUND)
  add_definitions(-DHAVE_PRISON)
endif()

add_subdirectory(icons)
add_subdirectory(printing)
add_subdirectory(tests)
if(QGPGME_FOUND)
  add_definitions(-DQGPGME_FOUND)
endif()

set(kaddressbook_printing_SRCS
  printing/detailled/detailledstyle.cpp
  printing/mike/mikesstyle.cpp
  printing/printingwizard.cpp
  printing/printprogress.cpp
  printing/printstyle.cpp
  printing/ringbinder/ringbinderstyle.cpp
  printing/compact/compactstyle.cpp
  printing/grantlee/grantleeprintstyle.cpp
  printing/grantlee/contactgrantleeprintobject.cpp
  printing/grantlee/contactgrantleeprintaddressobject.cpp
  printing/grantlee/contactgrantleeprintphoneobject.cpp
  printing/grantlee/contactgrantleeprintimobject.cpp
  printing/grantlee/contactgrantleeprintgeoobject.cpp
  printing/grantlee/contactgrantleeprintcryptoobject.cpp
  printing/stylepage.cpp
)

qt5_wrap_ui(kaddressbook_printing_SRCS
  printing/ringbinder/rbs_appearance.ui
  printing/detailled/ds_appearance.ui
  printing/compact/compactstyle.ui
)

set(kaddressbook_xxport_SRCS
  xxport/csv/csv_xxport.cpp
  xxport/csv/csvimportdialog.cpp
  xxport/csv/dateparser.cpp
  xxport/csv/qcsvmodel.cpp
  xxport/csv/qcsvreader.cpp
  xxport/csv/templateselectiondialog.cpp
  xxport/ldif/ldif_xxport.cpp
  xxport/ldap/ldap_xxport.cpp
  xxport/gmx/gmx_xxport.cpp
  xxport/shared/xxport.cpp
  xxport/vcard/vcard_xxport.cpp
  xxport/xxportfactory.cpp
)

set(kaddressbook_merge_SRCS
   merge/mergecontactsdialog.cpp
   merge/searchpotentialduplicatecontactjob.cpp 
   merge/mergecontactwidget.cpp
   merge/mergecontactutil.cpp
   merge/mergecontactsjob.cpp
   merge/mergecontactinfowidget.cpp
   merge/mergecontactwidgetlist.cpp
   merge/mergecontactshowresultdialog.cpp
   merge/mergecontactselectinformationdialog.cpp
   merge/mergecontactshowresulttabwidget.cpp
   merge/mergecontacts.cpp
   merge/mergecontactduplicatecontactdialog.cpp
)

set(kaddressbook_LIB_SRCS
  aboutdata.cpp
  categoryfilterproxymodel.cpp
  categoryselectwidget.cpp
  contactfields.cpp
  contactselectiondialog.cpp
  contactselectionwidget.cpp
  contactsorter.cpp
  contactswitcher.cpp
  globalcontactmodel.cpp
  mainwidget.cpp
  modelcolumnmanager.cpp
  quicksearchwidget.cpp
  xxportmanager.cpp
  utils.cpp
  ${kaddressbook_merge_SRCS}
  ${kaddressbook_printing_SRCS}
  ${kaddressbook_xxport_SRCS}
)

qt5_add_dbus_adaptor( kaddressbook_LIB_SRCS org.kde.kaddressbook.xml mainwidget.h MainWidget)

kconfig_add_kcfg_files(kaddressbook_LIB_SRCS settings/settings.kcfgc)
add_library(kaddressbookprivate ${LIBRARY_TYPE} ${kaddressbook_LIB_SRCS})
target_link_libraries(kaddressbookprivate
  kdepim
  pimcommon
  kaddressbookgrantlee
  KF5::AkonadiCore
  KF5::Abc
  KF5::AkonadiContact
  KF5::Ldap
  KF5::KCMUtils
  KF5::KontactInterface
  grantleetheme
  kleo
)
add_subdirectory(merge/tests)


if(QGPGME_FOUND)
  target_link_libraries(kaddressbookprivate ${QGPGME_LIBRARIES})
endif()

if(PRISON_FOUND)
  target_link_libraries(kaddressbookprivate ${PRISON_LIBRARIES})
endif()

set_target_properties(kaddressbookprivate PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS kaddressbookprivate ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

########### next target ###############

set(kaddressbook_SRCS
  main.cpp
  mainwindow.cpp
)

kde4_add_app_icon(kaddressbook_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-apps-kaddressbook.png")

add_executable(kaddressbook ${kaddressbook_SRCS})

target_link_libraries(kaddressbook
  KF5::AkonadiCore
  KF5::Syndication
  KF5::KCMUtils
  KF5::KIOCore
  KF5::KontactInterface
  kaddressbookprivate
  grantleetheme
)

if(QGPGME_FOUND)
  target_link_libraries(kaddressbook ${QGPGME_LIBRARIES})
endif()

install(TARGETS kaddressbook ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS data/kaddressbook.desktop data/kaddressbook-importer.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kaddressbookui.rc DESTINATION ${DATA_INSTALL_DIR}/kaddressbook)

set(kaddressbook_PART_SRCS kaddressbookpart.cpp)
add_library(kaddressbookpart MODULE ${kaddressbook_PART_SRCS})
target_link_libraries(kaddressbookpart ${KDE4_KPARTS_LIBS} kaddressbookprivate)
install(TARGETS kaddressbookpart DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES data/kaddressbookpart.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES
  viewertemplates/contact.html
  viewertemplates/contact_embedded.html
  viewertemplates/contact_row.html
  viewertemplates/contactgroup.html
  viewertemplates/contactgroup_embedded.html
  viewertemplates/contactgroup_additionalfieldrow.html
  viewertemplates/contactgroup_memberrow.html
  viewertemplates/theme.desktop
  DESTINATION ${DATA_INSTALL_DIR}/kaddressbook/viewertemplates/default/
)

install(FILES data/kaddressbook_themes.knsrc DESTINATION ${CONFIG_INSTALL_DIR} )

install(DIRECTORY printing/grantlee/themes/ DESTINATION ${DATA_INSTALL_DIR}/kaddressbook/printing/themes/ )
