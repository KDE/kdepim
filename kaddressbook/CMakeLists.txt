project(kaddressbook)
include_directories(
    ${CMAKE_SOURCE_DIR}/akonadi
    ${CMAKE_SOURCE_DIR}/akonadi/akonadi_next
    ${CMAKE_SOURCE_DIR}/libkdepim
    ${CMAKE_CURRENT_SOURCE_DIR}/xxport/shared
    ${QT_QTDBUS_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_subdirectory(icons)
add_subdirectory(printing)
if(QGPGME_FOUND)
  add_definitions(-DQGPGME_FOUND)
endif(QGPGME_FOUND)

set( kaddressbook_printing_SRCS
  printing/detailledstyle.cpp
  printing/kabentrypainter.cpp
  printing/mikesstyle.cpp
  printing/printingwizard.cpp
  printing/printprogress.cpp
  printing/printsortmode.cpp
  printing/printstyle.cpp
  printing/ringbinderstyle.cpp
  printing/selectionpage.cpp
  printing/stylepage.cpp
)

kde4_add_ui_files(kaddressbook_printing_SRCS
  printing/rbs_appearance.ui
  printing/ds_appearance.ui
)

set( kaddressbook_LIB_SRCS
  aboutdata.cpp
  contactfields.cpp
  contactfiltermodel.cpp
  contactstreemodel.cpp
  contactswitcher.cpp
  globalcontactmodel.cpp
  mainwidget.cpp
  modelcolumnmanager.cpp
  quicksearchwidget.cpp
  standardcontactactionmanager.cpp
  xxport/csv/csv_xxport.cpp
  xxport/csv/csvimportdialog.cpp
  xxport/csv/dateparser.cpp
  xxport/csv/qcsvmodel.cpp
  xxport/csv/qcsvreader.cpp
  xxport/ldif/ldif_xxport.cpp
  xxport/ldap/ldap_xxport.cpp
  xxport/ldap/ldapsearchdialog.cpp
  xxport/ldap/addhostdialog.cpp
  xxport/ldap/ldapoptionswidget.cpp
  xxport/gmx/gmx_xxport.cpp
  xxport/shared/xxport.cpp
  xxport/vcard/vcard_xxport.cpp 
  xxport/xxportfactory.cpp
  xxportmanager.cpp
  ${kaddressbook_printing_SRCS}
)

kde4_add_kcfg_files(kaddressbook_LIB_SRCS settings.kcfgc)
kde4_add_library(kaddressbookprivate SHARED ${kaddressbook_LIB_SRCS})
target_link_libraries(kaddressbookprivate kdepim ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_KABC_LIBS} ${KDEPIMLIBS_AKONADI_CONTACT_LIBS} ${KDEPIMLIBS_KLDAP_LIBS} ${KDE4_KUTILS_LIBS} akonadi_next)

if(QGPGME_FOUND)
  target_link_libraries(kaddressbookprivate ${QGPGME_LIBRARIES})
endif(QGPGME_FOUND)


set_target_properties(kaddressbookprivate PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kaddressbookprivate ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

########### next target ###############

set( kaddressbook_SRCS
  main.cpp
  mainwindow.cpp
)

# todo: more appropriate icon?
kde4_add_app_icon(kaddressbook_SRCS "${KDE4_ICON_DIR}/oxygen/*/actions/view-pim-contacts.png")

kde4_add_executable(kaddressbook RUN_UNINSTALLED ${kaddressbook_SRCS})

target_link_libraries(kaddressbook  ${KDEPIMLIBS_SYNDICATION_LIBS} kdepim ${KDEPIMLIBS_KCAL_LIBS}
                              ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_PHONON_LIBS})
target_link_libraries(kaddressbook ${KDEPIMLIBS_AKONADI_LIBS} ${KDE4_KDEUI_LIBS} kaddressbookprivate)

if(QGPGME_FOUND)
  target_link_libraries(kaddressbook ${QGPGME_LIBRARIES})
endif(QGPGME_FOUND)

install(TARGETS kaddressbook ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES kaddressbook.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kaddressbookui.rc  DESTINATION ${DATA_INSTALL_DIR}/kaddressbook)


set(kaddressbook_PART_SRCS kaddressbookpart.cpp )
kde4_add_plugin(kaddressbookpart ${kaddressbook_PART_SRCS})
target_link_libraries(kaddressbookpart ${KDE4_KPARTS_LIBS} kaddressbookprivate )
install(TARGETS kaddressbookpart DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kaddressbookpart.desktop DESTINATION ${SERVICES_INSTALL_DIR})
