project(akonotesplasmoid)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

set(akonotes_plasmoid_SRCS
    akonotes_applet.cpp
)

kde4_add_plugin(plasma_applet_akonotes ${akonotes_plasmoid_SRCS})

include_directories(
                      ${KDEPIMLIBS_INCLUDE_DIRS}
                      )

target_link_libraries(plasma_applet_akonotes
                      ${KDE4_PLASMA_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${KDE4_AKONADI_LIBS}
                      ${KDE4_AKONADI_KMIME_LIBS}
                      ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
                      ${KDE4_KMIME_LIBS}
)

target_link_libraries(plasma_applet_akonotes_list
                      ${KDE4_PLASMA_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${KDE4_AKONADI_LIBS}
                      ${KDE4_AKONADI_KMIME_LIBS}
                      ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
                      ${KDE4_KMIME_LIBS}
)

install(TARGETS plasma_applet_akonotes_note
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES akonotes.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})


