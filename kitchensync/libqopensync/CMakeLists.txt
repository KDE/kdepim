INCLUDE_DIRECTORIES( ${OPENSYNC_INCLUDE_DIRS} ${GLIB2_INCLUDE_DIR} )


########### next target ###############

SET(qopensync_LIB_SRCS
callbackhandler.cpp
engine.cpp
groupenv.cpp
pluginadvancedoption.cpp
pluginauthentication.cpp
pluginconnection.cpp
pluginenv.cpp
pluginlocalization.cpp
filter.cpp
group.cpp
member.cpp
plugin.cpp
result.cpp
syncmapping.cpp
syncupdates.cpp
syncchange.cpp
)


KDE4_ADD_LIBRARY(qopensync SHARED ${qopensync_LIB_SRCS})

TARGET_LINK_LIBRARIES(qopensync  ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${OPENSYNC_LDFLAGS})

SET_TARGET_PROPERTIES(qopensync PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
INSTALL(TARGETS qopensync DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############
