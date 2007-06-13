ADD_SUBDIRECTORY( about ) 
ADD_SUBDIRECTORY( pics ) 

INCLUDE_DIRECTORIES( ${OPENSYNC_INCLUDE_DIRS} ${OSENGINE_INCLUDE_DIRS} )


########### next target ###############

SET(kitchensync_SRCS
main.cpp
mainwindow.cpp
)

KDE4_AUTOMOC(${kitchensync_SRCS})

KDE4_ADD_EXECUTABLE(kitchensync ${kitchensync_SRCS})

TARGET_LINK_LIBRARIES(kitchensync kitchensyncprivate ${QT_QTGUI_LIBRARY} ${KDE_KDEUI_LIBRARY} )

INSTALL(TARGETS kitchensync DESTINATION ${BIN_INSTALL_DIR})


########### next target ###############

SET(kitchensyncprivate_LIB_SRCS
aboutpage.cpp
mainwidget.cpp
groupconfigdialog.cpp
groupconfig.cpp
groupitem.cpp
groupview.cpp
memberconfig.cpp
syncprocess.cpp
syncprocessmanager.cpp
connectionwidgets.cpp
pluginpicker.cpp
configgui.cpp
configguiblank.cpp
configguifile.cpp
memberinfo.cpp
groupconfigcommon.cpp
kwidgetlist.cpp
configguipalm.cpp
conflictdialog.cpp
singleconflictdialog.cpp
addresseediffalgo.cpp
calendardiffalgo.cpp
htmldiffalgodisplay.cpp
genericdiffalgo.cpp
multiconflictdialog.cpp
configguiirmc.cpp
configguisyncmlobex.cpp
configguisyncmlhttp.cpp
configguiopie.cpp
configguignokii.cpp
configguigcalendar.cpp
configguildap.cpp
configguigpe.cpp
)

KDE4_AUTOMOC(${kitchensyncprivate_LIB_SRCS})

KDE4_ADD_LIBRARY(kitchensyncprivate SHARED ${kitchensyncprivate_LIB_SRCS})

TARGET_LINK_LIBRARIES(kitchensyncprivate ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}
                                         ${KDE4_KDECORE_LIBRARY} ${KDE4_KDEUI_LIBRARY}
                                         ${KDE4_KDEKIO_LIBRARY} ${KDE4_KHTML_LIBRARY}
                                         kabc kcal kdepim qopensync )

SET_TARGET_PROPERTIES(kitchensyncprivate PROPERTIES VERSION 4.2.0 SOVERSION 4 )
INSTALL(TARGETS kitchensyncprivate DESTINATION ${LIB_INSTALL_DIR} )


########### next target ###############

SET(kitchensyncpart_PART_SRCS
part.cpp
)

KDE4_AUTOMOC(${kitchensyncpart_PART_SRCS})

KDE4_ADD_PLUGIN(kitchensyncpart WITH_PREFIX ${kitchensyncpart_PART_SRCS})

TARGET_LINK_LIBRARIES(kitchensyncpart kitchensyncprivate ${QT_AND_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} )

INSTALL(TARGETS kitchensyncpart DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

INSTALL( FILES kitchensync.desktop DESTINATION ${XDG_APPS_DIR} )
INSTALL( FILES kitchensyncui.rc kitchensync_part.rc DESTINATION ${DATA_INSTALL_DIR}/kitchensync )
