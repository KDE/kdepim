ADD_SUBDIRECTORY( about ) 
ADD_SUBDIRECTORY( pics ) 

INCLUDE_DIRECTORIES( ${OPENSYNC_INCLUDE_DIRS} ${GLIB2_INCLUDE_DIR} )


########### next target ###############

SET(kitchensync_SRCS
main.cpp
mainwindow.cpp
)


# todo: move to Oxygen icon
kde4_add_app_icon(kitchensync_SRCS "pics/hi*-app-kitchensync.png")

KDE4_ADD_EXECUTABLE(kitchensync ${kitchensync_SRCS})

TARGET_LINK_LIBRARIES(kitchensync kitchensyncprivate ${KDE4_KDEUI_LIBS} )

INSTALL(TARGETS kitchensync ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

SET(kitchensyncprivate_LIB_SRCS
aboutpage.cpp
addresseediffalgo.cpp
calendardiffalgo.cpp
conflictdialog.cpp
genericdiffalgo.cpp
groupconfig.cpp
groupconfigcommon.cpp
groupconfigdialog.cpp
groupitem.cpp
groupview.cpp
htmldiffalgodisplay.cpp
kwidgetlist.cpp
mainwidget.cpp
memberconfig.cpp
memberinfo.cpp
multiconflictdialog.cpp
pluginpicker.cpp
singleconflictdialog.cpp
syncprocess.cpp
syncprocessmanager.cpp

configwidget.cpp
configadvancedoptionwidget.cpp
configauthenticationwidget.cpp
configconnectionwidget.cpp
configlocalizationwidget.cpp
)


KDE4_ADD_LIBRARY(kitchensyncprivate SHARED ${kitchensyncprivate_LIB_SRCS})

TARGET_LINK_LIBRARIES(kitchensyncprivate ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}
                                         ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS}
                                         ${KDE4_KDEKIO_LIBS} ${KDE4_KHTML_LIBS}
                                         kabc kcal kdepim qopensync )

SET_TARGET_PROPERTIES(kitchensyncprivate PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
INSTALL(TARGETS kitchensyncprivate DESTINATION ${LIB_INSTALL_DIR} )


########### next target ###############

SET(kitchensyncpart_PART_SRCS
part.cpp
)


KDE4_ADD_PLUGIN(kitchensyncpart ${kitchensyncpart_PART_SRCS})

TARGET_LINK_LIBRARIES(kitchensyncpart kitchensyncprivate ${QT_AND_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} )

INSTALL(TARGETS kitchensyncpart DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

INSTALL( FILES kitchensync.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
INSTALL( FILES kitchensyncui.rc kitchensync_part.rc DESTINATION ${DATA_INSTALL_DIR}/kitchensync )
