project(kaddressbook)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/xxport/shared
  ${CMAKE_CURRENT_SOURCE_DIR}/printing/
  ${QT_QTDBUS_INCLUDE_DIR}
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


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")


if(Prison_FOUND)
  add_definitions(-DHAVE_PRISON)
  include_directories(${PRISON_INCLUDE_DIR})
endif()

add_subdirectory(icons)
add_subdirectory(printing)
add_subdirectory(tests)
if(QGPGME_FOUND)
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
  printing/grantlee/contactgrantleeprintobject.cpp
  printing/grantlee/contactgrantleeprintaddressobject.cpp
  printing/grantlee/contactgrantleeprintphoneobject.cpp
  printing/grantlee/contactgrantleeprintimobject.cpp
  printing/grantlee/contactgrantleeprintgeoobject.cpp
  printing/grantlee/contactgrantleeprintcryptoobject.cpp
  printing/grantlee/grantleeprint.cpp
  printing/stylepage.cpp
)

kde4_add_ui_files(kaddressbook_printing_SRCS
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
  xxport/xxportmanager.cpp
  xxport/vcard/vcardviewerdialog.cpp
  xxport/vcard/vcardexportselectionwidget.cpp
  xxport/contactselectiondialog.cpp
)

set(kaddressbook_merge_SRCS
   merge/job/searchpotentialduplicatecontactjob.cpp
   merge/job/mergecontactsjob.cpp
   merge/job/mergecontacts.cpp

   merge/manualmerge/mergecontactsdialog.cpp
   merge/manualmerge/mergecontactwidget.cpp
   merge/manualmerge/mergecontactwidgetlist.cpp

   merge/searchduplicate/searchandmergecontactduplicatecontactdialog.cpp
   merge/searchduplicate/searchduplicateresultwidget.cpp
   merge/searchduplicate/resultduplicatetreewidget.cpp
   merge/searchduplicate/mergecontactselectinformationtabwidget.cpp

   merge/widgets/mergecontactinfowidget.cpp
   merge/widgets/mergecontactshowresulttabwidget.cpp
   merge/widgets/mergecontactselectinformationwidget.cpp
   merge/widgets/mergecontactselectlistwidget.cpp
   merge/widgets/mergecontactloseinformationwarning.cpp
   merge/widgets/mergecontactselectinformationscrollarea.cpp
   merge/widgets/mergecontacterrorlabel.cpp
)

set(kaddressbook_gravatar_SRCS
   gravatar/gravatarcreatejob.cpp
   gravatar/gravatarupdatejob.cpp
)

set(kaddressbook_LIB_SRCS
  contactlist.cpp
  startup.cpp
  aboutdata.cpp
  categoryfilterproxymodel.cpp
  categoryselectwidget.cpp
  contactfields.cpp
  contactselectionwidget.cpp
  contactsorter.cpp
  contactswitcher.cpp
  globalcontactmodel.cpp
  mainwidget.cpp
  modelcolumnmanager.cpp
  quicksearchwidget.cpp
  utils.cpp
  ${kaddressbook_merge_SRCS}
  ${kaddressbook_printing_SRCS}
  ${kaddressbook_xxport_SRCS}
  ${kaddressbook_sendmail_SRCS}
  ${kaddressbook_sendvcards_SRCS}
  ${kaddressbook_gravatar_SRCS}
)

qt4_add_dbus_adaptor( kaddressbook_LIB_SRCS org.kde.kaddressbook.xml mainwidget.h MainWidget)

kde4_add_kcfg_files(kaddressbook_LIB_SRCS settings/settings.kcfgc)
kde4_add_library(kaddressbookprivate ${LIBRARY_TYPE} ${kaddressbook_LIB_SRCS})
target_link_libraries(kaddressbookprivate
  kdepim
  pimcommon
  kaddressbookgrantlee
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
  ${KDEPIMLIBS_KLDAP_LIBS}
  ${KDE4_KCMUTILS_LIBS}
  ${KDE4_KPRINTUTILS_LIBS}
  ${Grantlee_CORE_LIBRARIES}
  grantleetheme
  kleo
)
add_subdirectory(merge/tests)
add_subdirectory(sendmail/tests)
add_subdirectory(sendvcards/autotests)
add_subdirectory(printing/grantlee/autotests)
add_subdirectory(gravatar/autotests)

if(QGPGME_FOUND)
  target_link_libraries(kaddressbookprivate ${QGPGME_LIBRARIES})
endif()

if(PRISON_FOUND)
  target_link_libraries(kaddressbookprivate ${PRISON_LIBRARIES})
endif()

set_target_properties(kaddressbookprivate PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kaddressbookprivate ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

########### next target ###############

set(kaddressbook_SRCS
  main.cpp
  mainwindow.cpp
)

kde4_add_app_icon(kaddressbook_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-apps-kaddressbook.png")

kde4_add_executable(kaddressbook ${kaddressbook_SRCS})

target_link_libraries(kaddressbook
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_SYNDICATION_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KCMUTILS_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDE4_PHONON_LIBS}
  ${KDEPIMLIBS_KONTACTINTERFACE_LIBS}
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
kde4_add_plugin(kaddressbookpart ${kaddressbook_PART_SRCS})
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
