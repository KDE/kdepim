SET(STANDALONE_SRCS	standalonenoteeditor.cpp
			standalonenoteitem.cpp
			standalonenotewidget.cpp
)

SET(STANDALONE_HDRS	include/StickyNotes/StandaloneNoteItem
			include/standalonenoteeditor.h
			include/standalonenoteitem.h
			include/standalonenotewidget.h
)

SET(STANDALONE_UIS	standalonenoteeditor.ui)

SET(STANDALONE_RCS	standalonenoteeditor.qrc
			standalonenotewidget.qrc)

INCLUDE_DIRECTORIES(BEFORE SYSTEM ${CMAKE_CURRENT_BINARY_DIR}
				  ${CMAKE_CURRENT_BINARY_DIR}/include
				  ${CMAKE_CURRENT_SOURCE_DIR}/include)

QT4_ADD_RESOURCES(STANDALONE_RC_SRCS ${STANDALONE_RCS})
KDE4_ADD_UI_FILES(STANDALONE_UI_SRCS ${STANDALONE_UIS})

KDE4_ADD_LIBRARY(stickynotes-standalone	${STANDALONE_SRCS}
					${STANDALONE_RC_SRCS}
					${STANDALONE_UI_SRCS})

TARGET_LINK_LIBRARIES(stickynotes-standalone	stickynotes
						${KDE4_KDECORE_LIBS}
						${QT_QTCORE_LIBRARY}
						${QT_QTGUI_LIBRARY}
						${QT_QTSVG_LIBRARY}
						${QT_QTXML_LIBRARY})

install(TARGETS stickynotes-standalone ${INSTALL_TARGETS_DEFAULT_ARGS})

#INSTALL(FILES ${STANDALONE_HDRS}
#	DESTINATION ${INCLUDE_INSTALL_DIR}/StickyNotes)


ADD_SUBDIRECTORY(demo)

