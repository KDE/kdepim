project(akonotesplasmoid)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

set(akonotes_note_plasmoid_SRCS
    akonotes_noteapplet.cpp
)

set(akonotes_noteslist_plasmoid_SRCS
    akonotes_noteslistapplet.cpp
    ../kjotsmodel.cpp
    plasmatreeview.cpp
)

kde4_add_ui_files(akonotes_noteslist_plasmoid_SRCS akonotes_applet.ui )

kde4_add_plugin(plasma_applet_akonotes_note ${akonotes_note_plasmoid_SRCS})
kde4_add_plugin(plasma_applet_akonotes_list ${akonotes_noteslist_plasmoid_SRCS})

include_directories(
                      ${CMAKE_CURRENT_SOURCE_DIR}/..
                      ${KDEPIMLIBS_INCLUDE_DIRS}
                      )

target_link_libraries(plasma_applet_akonotes_note
                      ${KDE4_PLASMA_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${KDEPIMLIBS_AKONADI_LIBS}
                      ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
                      ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
                      ${KDEPIMLIBS_KMIME_LIBS}
                      akonadi_next
                      noteshared
)

target_link_libraries(plasma_applet_akonotes_list
                      ${KDE4_PLASMA_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${KDEPIMLIBS_AKONADI_LIBS}
                      ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
                      ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
                      ${Grantlee_GUI_LIBRARIES}
                      ${KDEPIMLIBS_KMIME_LIBS}
                      akonadi_next
                      noteshared
)

install(TARGETS plasma_applet_akonotes_note
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES akonotes_note.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(TARGETS plasma_applet_akonotes_list
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES akonotes_list.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})


install(FILES stickynote.svgz
  DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)


