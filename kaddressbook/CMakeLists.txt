project(kcontactmanager)
include_directories(
    ${CMAKE_SOURCE_DIR}/akonadi
    ${CMAKE_SOURCE_DIR}/akonadi/akonadi_next
    ${CMAKE_SOURCE_DIR}/libkdepim
    ${CMAKE_CURRENT_SOURCE_DIR}/editor
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

set( kcontactmanager_printing_SRCS
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

kde4_add_ui_files(kcontactmanager_printing_SRCS
  printing/rbs_appearance.ui
  printing/ds_appearance.ui
)

set( kcontactmanager_editor_SRCS
  editor/addresseditwidget.cpp
  editor/contacteditor.cpp
  editor/dateeditwidget.cpp
  editor/displaynameeditwidget.cpp
  editor/emaileditwidget.cpp
  editor/freebusyeditwidget.cpp
  editor/imagewidget.cpp
  editor/imeditwidget.cpp
  editor/nameeditwidget.cpp
  editor/phoneeditwidget.cpp
  editor/secrecyeditwidget.cpp
  editor/soundeditwidget.cpp
)

set( kcontactmanager_LIB_SRCS
  aboutdata.cpp
  contactfields.cpp
  contacteditordialog.cpp
  contactfiltermodel.cpp
  contactgroupeditordialog.cpp
  contactitemeditor.cpp
  contactmetadata.cpp
  contactstreemodel.cpp
  collectioncombobox.cpp
  collectionselectiondialog.cpp
  globalcontactmodel.cpp
  mainwidget.cpp
  quicksearchwidget.cpp
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
  xxport/shared/xxport.cpp
  xxport/vcard/vcard_xxport.cpp 
  xxport/xxportfactory.cpp
  xxportmanager.cpp
  ${kcontactmanager_editor_SRCS}
  ${kcontactmanager_printing_SRCS}
)

qt4_add_dbus_adaptor(kcontactmanager_LIB_SRCS org.kde.KContactmanager.MainWidget.xml mainwidget.h MainWidget kcontactmanageradaptor )

kde4_add_library(kcontactmanagerprivate SHARED ${kcontactmanager_LIB_SRCS})
target_link_libraries(kcontactmanagerprivate kdepim ${KDE4_PHONON_LIBS} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_KABC_LIBS} ${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_KLDAP_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} akonadi-kabccommon akonadi_next)

if(QGPGME_FOUND)
  target_link_libraries(kcontactmanagerprivate ${QGPGME_LIBRARIES})
endif(QGPGME_FOUND)


set_target_properties(kcontactmanagerprivate PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kcontactmanagerprivate ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

########### next target ###############

set( kcontactmanager_SRCS
  main.cpp
  mainwindow.cpp
)

# todo: more appropriate icon?
kde4_add_app_icon(kcontactmanager_SRCS "${KDE4_ICON_DIR}/oxygen/*/actions/view-pim-contacts.png")

kde4_add_executable(kcontactmanager RUN_UNINSTALLED ${kcontactmanager_SRCS})

target_link_libraries(kcontactmanager  ${KDEPIMLIBS_SYNDICATION_LIBS} kdepim ${KDEPIMLIBS_KCAL_LIBS}
                              ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_PHONON_LIBS})
target_link_libraries(kcontactmanager ${KDEPIMLIBS_AKONADI_LIBS} akonadi-kabccommon ${KDE4_KDEUI_LIBS} kcontactmanagerprivate)

if(QGPGME_FOUND)
  target_link_libraries(kcontactmanager ${QGPGME_LIBRARIES})
endif(QGPGME_FOUND)

install(TARGETS kcontactmanager ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES kcontactmanager.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kcontactmanagerui.rc  DESTINATION ${DATA_INSTALL_DIR}/kcontactmanager)


set(kcontactmanager_PART_SRCS kcontactmanager_part.cpp )
kde4_add_plugin(kcontactmanagerpart ${kcontactmanager_PART_SRCS})
target_link_libraries(kcontactmanagerpart ${KDE4_KPARTS_LIBS} kcontactmanagerprivate )
install(TARGETS kcontactmanagerpart DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES kcontactmanagerpart.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kcontactmanager_part.rc DESTINATION ${DATA_INSTALL_DIR}/kcontactmanager)

