project(kaddressbook)


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


if(KF5Prison_FOUND)
  add_definitions(-DHAVE_PRISON)
endif()

add_subdirectory(icons)
add_subdirectory(printing)
if (BUILD_TESTING)
   add_subdirectory(autotests)
endif()

if(KF5Gpgmepp_FOUND)
  add_definitions(-DQGPGME_FOUND)
endif()

set(kaddressbook_sendmail_SRCS
  sendmail/mailsenderjob.cpp
)

set(kaddressbook_sendvcards_SRCS
  sendvcards/sendvcardsjob.cpp
)

set(kaddressbook_printing_SRCS
  printing/detailled/detailledstyle.cpp
  printing/mike/mikesstyle.cpp
  printing/printingwizard.cpp
  printing/printprogress.cpp
  printing/printstyle.cpp
  printing/ringbinder/ringbinderstyle.cpp
  printing/compact/compactstyle.cpp
  printing/grantlee/grantleeprintstyle.cpp
  printing/stylepage.cpp
)

ki18n_wrap_ui(kaddressbook_printing_SRCS
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
   merge/job/searchpotentialduplicatecontactjob.cpp
   merge/job/mergecontactsjob.cpp
   merge/job/mergecontacts.cpp
   merge/mergecontactsdialog.cpp
   merge/mergecontactwidget.cpp
   merge/util/mergecontactutil.cpp
   merge/mergecontactinfowidget.cpp
   merge/mergecontactwidgetlist.cpp
   merge/mergecontactshowresultdialog.cpp
   merge/mergecontactselectinformationdialog.cpp
   merge/mergecontactshowresulttabwidget.cpp
   merge/searchduplicate/searchandmergecontactduplicatecontactdialog.cpp
)

set(kaddressbook_LIB_SRCS
  kaddressbook_debug.cpp
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
  ${kaddressbook_sendmail_SRCS}
  ${kaddressbook_sendvcards_SRCS}
)

qt5_add_dbus_adaptor( kaddressbook_LIB_SRCS org.kde.kaddressbook.xml mainwidget.h MainWidget)

kconfig_add_kcfg_files(kaddressbook_LIB_SRCS settings/settings.kcfgc)
add_library(kaddressbookprivate ${LIBRARY_TYPE} ${kaddressbook_LIB_SRCS})
generate_export_header(kaddressbookprivate BASE_NAME kaddressbook)
target_link_libraries(kaddressbookprivate
  kdepim
  pimcommon
  kaddressbookgrantlee
  KF5::AkonadiCore
  KF5::Contacts
  KF5::AkonadiContact
  KF5::Ldap
  KF5::KCMUtils
  KF5::KontactInterface
  KF5::Codecs
  grantleetheme
  kleo
)
if (BUILD_TESTING)
   add_subdirectory(merge/autotests)
   add_subdirectory(sendmail/autotests)
   add_subdirectory(sendvcards/autotests)
endif()

if(KF5Gpgmepp_FOUND)
   target_link_libraries(kaddressbookprivate KF5::QGpgme)
endif()
if(KF5Prison_FOUND)
  target_link_libraries(kaddressbookprivate KF5::Prison)
endif()

set_target_properties(kaddressbookprivate PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS kaddressbookprivate ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

########### next target ###############

set(kaddressbook_SRCS
  kaddressbook_debug.cpp
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

if(KF5Gpgmepp_FOUND)
  target_link_libraries(kaddressbook KF5::QGpgme)
endif()

install(TARGETS kaddressbook ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS data/org.kde.kaddressbook.desktop data/kaddressbook-importer.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES data/kaddressbook.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES kaddressbookui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kaddressbook)

set(kaddressbook_PART_SRCS kaddressbookpart.cpp kaddressbook_debug.cpp)
add_library(kaddressbookpart MODULE ${kaddressbook_PART_SRCS})
target_link_libraries(kaddressbookpart  kaddressbookprivate)
install(TARGETS kaddressbookpart DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES data/kaddressbookpart.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

install(FILES
  viewertemplates/contact.html
  viewertemplates/contact_embedded.html
  viewertemplates/contact_row.html
  viewertemplates/contactgroup.html
  viewertemplates/contactgroup_embedded.html
  viewertemplates/contactgroup_additionalfieldrow.html
  viewertemplates/contactgroup_memberrow.html
  viewertemplates/theme.desktop
  DESTINATION ${KDE_INSTALL_DATADIR}/kaddressbook/viewertemplates/default/
)

install(FILES data/kaddressbook_themes.knsrc DESTINATION ${KDE_INSTALL_CONFDIR} )

install(DIRECTORY printing/grantlee/themes/ DESTINATION ${KDE_INSTALL_DATADIR}/kaddressbook/printing/themes/ )
