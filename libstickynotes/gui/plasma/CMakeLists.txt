
SET(PLASMANOTE_SRCS	stickynote.cpp
			stickynoteconnection.cpp
			stickynoteeditor.cpp
			stickynoteitem.cpp
			stickynotewidget.cpp)

SET(PLASMANOTE_UIS	stickynoteeditor.ui)

SET(PLASMANOTE_RCS	stickynoteeditor.qrc)

QT4_ADD_RESOURCES(PLASMANOTE_RC_SRCS ${PLASMANOTE_RCS})
kde4_add_ui_files(PLASMANOTE_UI_SRCS ${PLASMANOTE_UIS})

KDE4_ADD_PLUGIN(plasma_applet_stickynote	${PLASMANOTE_SRCS}
						${PLASMANOTE_RC_SRCS}
						${PLASMANOTE_UI_SRCS})


TARGET_LINK_LIBRARIES(plasma_applet_stickynote	stickynotes
						${KDE4_PLASMA_LIBS}
						${KDE4_KDEUI_LIBS}
						${QT_QTNETWORK_LIBRARY})


INSTALL(TARGETS plasma_applet_stickynote
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES plasma-applet-stickynote.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

INSTALL(FILES stickynote.svgz
	DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

ADD_SUBDIRECTORY(demo)

