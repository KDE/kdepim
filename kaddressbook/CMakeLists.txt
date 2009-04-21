project(kcontactmanager)
include_directories(
    ${CMAKE_SOURCE_DIR}/akonadi
    ${CMAKE_SOURCE_DIR}/libkdepim
    ${CMAKE_CURRENT_SOURCE_DIR}/editor
    ${CMAKE_CURRENT_SOURCE_DIR}/xxport/shared
    ${QT_QTDBUS_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_subdirectory(icons)

if(QGPGME_FOUND)
  add_definitions(-DQGPGME_FOUND)
endif(QGPGME_FOUND)

set( kcontactmanager_editor_SRCS
  editor/contacteditor.cpp
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
  mainwidget.cpp
  contacteditordialog.cpp
  contactgroupeditordialog.cpp
  xxportmanager.cpp
  contactfiltermodel.cpp
  quicksearchwidget.cpp
  collectionselectiondialog.cpp
  xxport/xxportfactory.cpp
  contactitemeditor.cpp
  collectioncombobox.cpp
  xxport/csv/csv_xxport.cpp
  xxport/csv/csvimportdialog.cpp
  xxport/csv/dateparser.cpp
  xxport/csv/qcsvmodel.cpp
  xxport/csv/qcsvreader.cpp
  xxport/ldif/ldif_xxport.cpp
  xxport/shared/xxport.cpp
  xxport/vcard/vcard_xxport.cpp 
  contactfields.cpp
  contactmetadata.cpp
  ${kcontactmanager_editor_SRCS}
  )

kde4_add_library(kcontactmanagerprivate SHARED ${kcontactmanager_LIB_SRCS})
target_link_libraries(kcontactmanagerprivate kdepim ${KDE4_PHONON_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_AKONADI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} akonadi-kabccommon)

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

target_link_libraries(kcontactmanager  ${KDE4_SYNDICATION_LIBS} kdepim ${KDE4_KCAL_LIBS}
                              ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_PHONON_LIBS})
target_link_libraries(kcontactmanager ${KDE4_AKONADI_LIBS} akonadi-kabccommon ${KDE4_KDEUI_LIBS} kcontactmanagerprivate)

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

