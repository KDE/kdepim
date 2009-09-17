FIND_PACKAGE(KDE4 REQUIRED)

INCLUDE(KDE4Defaults)
 
ADD_DEFINITIONS(${KDE4_DEFINITIONS})

INCLUDE_DIRECTORIES(BEFORE SYSTEM ${KDE4_INCLUDES}
				  ${CMAKE_BINARY_DIR}/gui/plasma/include
				  ${CMAKE_SOURCE_DIR}/gui/plasma/include)


SET(PLASMANOTE_SRCS	stickynote.cpp
			stickynoteeditor.cpp
			stickynoteitem.cpp
			stickynotewidget.cpp)

SET(PLASMANOTE_HDRS	stickynote.h
			stickynoteeditor.h
			stickynoteitem.h
			stickynotewidget.h)

SET(PLASMANOTE_MOC_HDRS		stickynote.h
				stickynoteeditor.h
				stickynoteitem.h
				stickynotewidget.h)

SET(PLASMANOTE_UIS	stickynoteeditor.ui)

SET(PLASMANOTE_RCS	stickynoteeditor.qrc)

QT4_ADD_RESOURCES(PLASMANOTE_RC_SRCS ${PLASMANOTE_RCS})
QT4_WRAP_CPP(PLASMANOTE_MOC_SRCS ${PLASMANOTE_MOC_HDRS})
QT4_WRAP_UI(PLASMANOTE_UI_HDRS ${PLASMANOTE_UIS})
ADD_CUSTOM_TARGET(stickynote_qt4 DEPENDS	${PLASMANOTE_MOC_SRCS}
						${PLASMANOTE_UI_HDRS})



KDE4_ADD_PLUGIN(plasma_applet_stickynote	${PLASMANOTE_SRCS}
						${PLASMANOTE_RC_SRCS})

ADD_DEPENDENCIES(plasma_applet_stickynote stickynote_qt4)


TARGET_LINK_LIBRARIES(plasma_applet_stickynote	stickynotes
						${KDE4_PLASMA_LIBS}
						${KDE4_KDEUI_LIBS})


INSTALL(TARGETS plasma_applet_stickynote
        DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES plasma-applet-stickynote.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

INSTALL(FILES stickynote.svgz
	DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

ADD_SUBDIRECTORY(demo)

