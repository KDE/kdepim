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

########### next target ###############

set( kcontactmanager_SRCS
  collectioncombobox.cpp
  collectionselectiondialog.cpp
  contacteditordialog.cpp
  contactfiltermodel.cpp
  contactgroupeditordialog.cpp
  contactitemeditor.cpp
  contactmetadata.cpp
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
  main.cpp
  mainwidget.cpp
  mainwindow.cpp
  quicksearchwidget.cpp
  xxport/opera/opera_xxport.cpp
  xxport/ldif/ldif_xxport.cpp
  xxport/shared/xxport.cpp
  xxport/vcard/vcard_xxport.cpp
  xxport/eudora/eudora_xxport.cpp
  xxport/xxportfactory.cpp
  xxportmanager.cpp
)

# todo: more appropriate icon?
kde4_add_app_icon(kcontactmanager_SRCS "${KDE4_ICON_DIR}/oxygen/*/actions/view-pim-contacts.png")

kde4_add_executable(kcontactmanager RUN_UNINSTALLED ${kcontactmanager_SRCS})

target_link_libraries(kcontactmanager  ${KDE4_SYNDICATION_LIBS} kdepim ${KDE4_KCAL_LIBS}
                              ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_PHONON_LIBS})
target_link_libraries(kcontactmanager ${KDE4_AKONADI_LIBS} akonadi-kabccommon ${KDE4_KDEUI_LIBS} )

if(QGPGME_FOUND)
  target_link_libraries(kcontactmanager ${QGPGME_LIBRARIES})
endif(QGPGME_FOUND)

install(TARGETS kcontactmanager ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES kcontactmanager.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kcontactmanagerui.rc  DESTINATION ${DATA_INSTALL_DIR}/kcontactmanager)
