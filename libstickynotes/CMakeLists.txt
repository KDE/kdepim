PROJECT(stickynotes)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(QT_USE_QTGUI FALSE)

SET(BUILD_SHARED_LIBS TRUE CACHE BOOL "Make library shared")
SET(BUILD_GUI TRUE CACHE BOOL "Build graphical user interfaces")
SET(BUILD_STANDALONE_GUI TRUE CACHE BOOL "Build standalone GUI")
SET(BUILD_PLASMA_GUI TRUE CACHE BOOL "Build plasma GUI")

IF(BUILD_SHARED_LIBS)
  IF(WIN32)
    SET(LSN_EXPORT "(__declspec(dllexport)")
    SET(LSN_IMPORT "(__declspec(dllimport)")
  ENDIF(WIN32)
ENDIF(BUILD_SHARED_LIBS)

IF(BUILD_GUI)
    IF(BUILD_STANDALONE_GUI)
        SET(LSN_STANDALONE_GUI_AVAILABLE TRUE)
	SET(QT_USE_QTGUI TRUE)
        SET(QT_USE_QTSVG TRUE)
        SET(QT_USE_QTXML TRUE)
    ENDIF(BUILD_STANDALONE_GUI)

    IF(BUILD_PLASMA_GUI)
        SET(LSN_PLASMA_GUI_AVAILABLE TRUE)
	SET(QT_USE_QTGUI TRUE)
        SET(QT_USE_QTSVG TRUE)
        SET(QT_USE_QTXML TRUE)
    ENDIF(BUILD_PLASMA_GUI)
ELSE(BUILD_GUI)
	UNSET(BUILD_STANDALONE_GUI CACHE)
	UNSET(BUILD_PLASMA_GUI CACHE)
ENDIF(BUILD_GUI)

CONFIGURE_FILE(config-stickynotes.h.cmake include/StickyNotes/config-stickynotes.h)


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
			${CMAKE_BINARY_DIR}/include/StickyNotes/config.h)

SET(STICKYNOTES_MOC_HDRS	include/hooknoteitem.h
				include/remotenotecontroller.h)


SET(QT_USE_QTNETWORK TRUE)

FIND_PACKAGE(Qt4 REQUIRED)

INCLUDE(${QT_USE_FILE})

ADD_DEFINITIONS(-Wall ${QT_DEFINITIONS})


INCLUDE_DIRECTORIES(BEFORE SYSTEM   ${CMAKE_BINARY_DIR}
				    ${CMAKE_BINARY_DIR}/include
				    ${CMAKE_SOURCE_DIR}/include)


QT4_WRAP_CPP(STICKYNOTES_MOC_SRCS ${STICKYNOTES_MOC_HDRS})

ADD_CUSTOM_TARGET(stickynotes_qt4 DEPENDS ${STICKYNOTES_MOC_SRCS})


ADD_LIBRARY(stickynotes	${STICKYNOTES_SRCS}
			${STICKYNOTES_RC_SRCS})

ADD_DEPENDENCIES(stickynotes stickynotes_qt4)

TARGET_LINK_LIBRARIES(stickynotes   ${QT_QTCORE_LIBRARY}
				    ${QT_QTNETWORK_LIBRARY})


INSTALL(TARGETS stickynotes 
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE	DESTINATION lib)

INSTALL(FILES ${STICKYNOTES_HDRS}
	DESTINATION include/StickyNotes)

ADD_SUBDIRECTORY(demo)

IF(BUILD_GUI)
    ADD_SUBDIRECTORY(gui)
ENDIF(BUILD_GUI)

