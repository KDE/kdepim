INCLUDE_DIRECTORIES( ${OPENSYNC_INCLUDE_DIRS} ${OSENGINE_INCLUDE_DIRS} )


########### next target ###############

SET(qopensync_LIB_SRCS
callbackhandler.cpp
conversion.cpp
engine.cpp
environment.cpp
filter.cpp
group.cpp
member.cpp
plugin.cpp
result.cpp
syncmapping.cpp
syncupdates.cpp
syncchange.cpp
)

KDE4_AUTOMOC(${qopensync_LIB_SRCS})

KDE4_ADD_LIBRARY(qopensync SHARED ${qopensync_LIB_SRCS})

TARGET_LINK_LIBRARIES(qopensync  ${QT_AND_KDECORE_LIBS} kdeui ${OPENSYNC_LDFLAGS} ${OSENGINE_LDFLAGS} )

SET_TARGET_PROPERTIES(qopensync PROPERTIES VERSION 4.2.0 SOVERSION 4 )
INSTALL(TARGETS qopensync DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kitchensync/src \
#           -I$(top_srcdir)/kitchensync \
#           -I$(top_srcdir) \
#           $(OPENSYNC_CFLAGS) \
#           $(OPENSYNCENGINE_CFLAGS) \
#           $(all_includes)
#
#lib_LTLIBRARIES = libqopensync.la
#
#libqopensync_la_SOURCES = callbackhandler.cpp conversion.cpp engine.cpp environment.cpp filter.cpp group.cpp \
#	member.cpp plugin.cpp result.cpp syncmapping.cpp syncupdates.cpp \
#	syncchange.cpp
#libqopensync_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
#libqopensync_la_LIBADD = $(LIB_KDEUI) $(OPENSYNC_LIBS) $(OPENSYNCENGINE_LIBS)
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o $(podir)/libqopensync.pot
