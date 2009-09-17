SET(STANDALONE_SRCS	standalonenoteeditor.cpp
			standalonenoteitem.cpp
			standalonenotewidget.cpp
)

SET(STANDALONE_HDRS	include/StickyNotes/StandaloneNoteItem
			include/standalonenoteeditor.h
			include/standalonenoteitem.h
			include/standalonenotewidget.h
)

SET(STANDALONE_MOC_HDRS	include/standalonenoteeditor.h
			include/standalonenoteitem.h
			include/standalonenotewidget.h
)

SET(STANDALONE_UIS	standalonenoteeditor.ui)

SET(STANDALONE_RCS	standalonenoteeditor.qrc
			standalonenotewidget.qrc)

INCLUDE_DIRECTORIES(BEFORE SYSTEM ${CMAKE_BINARY_DIR}/gui/standalone
				  ${CMAKE_BINARY_DIR}/gui/standalone/include
				  ${CMAKE_SOURCE_DIR}/gui/standalone/include)

QT4_ADD_RESOURCES(STANDALONE_RC_SRCS ${STANDALONE_RCS})
QT4_WRAP_CPP(STANDALONE_MOC_SRCS ${STANDALONE_MOC_HDRS})
QT4_WRAP_UI(STANDALONE_UI_HDRS ${STANDALONE_UIS})
ADD_CUSTOM_TARGET(standalone_qt4 DEPENDS	${STANDALONE_MOC_SRCS}
						${STANDALONE_UI_HDRS})

ADD_LIBRARY(stickynotes-standalone	${STANDALONE_SRCS}
					${STANDALONE_RC_SRCS})

ADD_DEPENDENCIES(stickynotes-standalone standalone_qt4)

TARGET_LINK_LIBRARIES(stickynotes-standalone	stickynotes
						${QT_QTGUI_LIBRARY}
						${QT_QTSVG_LIBRARY}
						${QT_QTXML_LIBRARY})


INSTALL(TARGETS stickynotes-standalone 
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE	DESTINATION lib)

INSTALL(FILES ${STANDALONE_HDRS}
	DESTINATION include/StickyNotes)


ADD_SUBDIRECTORY(demo)

