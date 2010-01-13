PROJECT(stickynotes)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}/include)

option(STICKYNOTES_BUILD_GUI "Build graphical user interfaces" TRUE)
option(STICKYNOTES_BUILD_STANDALONE_GUI "Build standalone GUI" FALSE)
option(STICKYNOTES_BUILD_PLASMA_GUI "Build plasma GUI" TRUE)
option(STICKYNOTES_BUILD_SHARED_LIBS "Make library shared" TRUE)

IF(STICKYNOTES_BUILD_GUI)
    IF(STICKYNOTES_BUILD_STANDALONE_GUI)
        SET(LSN_STANDALONE_GUI_AVAILABLE TRUE)
	SET(QT_USE_QTGUI TRUE)
        SET(QT_USE_QTSVG TRUE)
        SET(QT_USE_QTXML TRUE)
    ENDIF(STICKYNOTES_BUILD_STANDALONE_GUI)

    IF(STICKYNOTES_BUILD_PLASMA_GUI)
        SET(LSN_PLASMA_GUI_AVAILABLE TRUE)
	SET(QT_USE_QTGUI TRUE)
        SET(QT_USE_QTSVG TRUE)
        SET(QT_USE_QTXML TRUE)
    ENDIF(STICKYNOTES_BUILD_PLASMA_GUI)
ELSE(STICKYNOTES_BUILD_GUI)
	SET(STICKYNOTES_BUILD_STANDALONE_GUI FALSE)
	SET(STICKYNOTES_BUILD_PLASMA_GUI FALSE)
ENDIF(STICKYNOTES_BUILD_GUI)

CONFIGURE_FILE(config-stickynotes.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/include/StickyNotes/config-stickynotes.h)


SET(STICKYNOTES_SRCS	src/basenoteitem.cpp
			src/cachenoteitem.cpp
			src/hooknoteitem.cpp
			src/memorynoteitem.cpp
			src/remotenotecontroller.cpp
			src/remotenoteitem.cpp
			src/remotenotemessage.cpp
			src/remoteservicemessage.cpp)

SET(STICKYNOTES_HDRS	include/StickyNotes/AbstractNoteItem
			include/StickyNotes/BaseNoteItem
			include/StickyNotes/CacheNoteItem
			include/StickyNotes/HookNoteItem
			include/StickyNotes/MemoryNoteItem
			include/StickyNotes/RemoteNoteController
			include/StickyNotes/RemoteNoteItem
			include/StickyNotes/RemoteNoteMessage
			include/StickyNotes/RemoteServiceMessage
			include/StickyNotes/global.h
			include/abstractnoteitem.h
			include/basenoteitem.h
			include/cachenoteitem.h
			include/hooknoteitem.h
			include/memorynoteitem.h
			include/remotenotecontroller.h
			include/remotenoteitem.h
			include/remotenotemessage.h
			include/remoteservicemessage.h
			${CMAKE_CURRENT_BINARY_DIR}/include/StickyNotes/config-stickynotes.h)


KDE4_ADD_LIBRARY(stickynotes	${STICKYNOTES_SRCS}
				${STICKYNOTES_RC_SRCS})


target_link_libraries(stickynotes ${KDE4_KDEUI_LIBS} )

install(TARGETS stickynotes ${INSTALL_TARGETS_DEFAULT_ARGS})

#INSTALL(FILES ${STICKYNOTES_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/StickyNotes)

ADD_SUBDIRECTORY(demo)

IF(STICKYNOTES_BUILD_GUI)
    ADD_SUBDIRECTORY(gui)
ENDIF(STICKYNOTES_BUILD_GUI)

