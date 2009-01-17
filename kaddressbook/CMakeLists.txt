include_directories(
    ${CMAKE_SOURCE_DIR}/akonadi
    ${CMAKE_SOURCE_DIR}/libkdepim
    ${CMAKE_CURRENT_SOURCE_DIR}/editor
    ${QT_QTDBUS_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_subdirectory(icons)

########### next target ###############

set( kcontactmanager_SRCS
  collectioncombobox.cpp
  contacteditordialog.cpp
  contactitemeditor.cpp
  contactgroupeditordialog.cpp
  main.cpp
  mainwidget.cpp
  mainwindow.cpp

  # editors/addresseditwidget.cpp
  # editors/addresseeconfig.cpp
  # editors/addresseeeditorwidget.cpp
  # editors/advancedcustomfields.cpp
  # editors/contacteditorwidget.cpp
  # editors/contacteditorwidgetmanager.cpp
  # editors/customfieldswidget.cpp
  # editors/emaileditwidget.cpp
  # editors/extendedcontacteditor.cpp
  # editors/extendedcontacteditordialog.cpp
  # editors/freebusywidget.cpp
  # editors/geowidget.cpp
  # editors/imagewidget.cpp
  # editors/imeditwidget.cpp
  # editors/keywidget.cpp
  # editors/nameeditdialog.cpp
  # editors/phoneeditwidget.cpp
  # editors/secrecywidget.cpp
  # editors/soundwidget.cpp
   editor/addresseditwidget.cpp
   editor/contacteditor.cpp
   editor/emaileditwidget.cpp
   editor/imagewidget.cpp
   editor/soundeditwidget.cpp
)

# todo: more appropriate icon?
kde4_add_app_icon(kcontactmanager_SRCS "${KDE4_ICON_DIR}/oxygen/*/actions/view-pim-contacts.png")

kde4_add_executable(kcontactmanager RUN_UNINSTALLED ${kcontactmanager_SRCS})

kdepim4_link_unique_libraries(kcontactmanager  ${KDE4_SYNDICATION_LIBS} kdepim ${KDE4_KCAL_LIBS}
                              ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_PHONON_LIBS})
target_link_libraries(kcontactmanager ${KDE4_AKONADI_LIBS} akonadi-kabccommon ${KDE4_KDEUI_LIBS} )

install(TARGETS kcontactmanager ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES kcontactmanager.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kcontactmanagerui.rc  DESTINATION ${DATA_INSTALL_DIR}/kcontactmanager)
