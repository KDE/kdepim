INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/kitchensync ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(opensyncdbus_SRCS
dbusclient.cpp
)

KDE3_AUTOMOC(${opensyncdbus_SRCS})

KDE3_ADD_EXECUTABLE(opensyncdbus ${opensyncdbus_SRCS})

TARGET_LINK_LIBRARIES(opensyncdbus  ${QT_AND_KDECORE_LIBS} qopensync dbus-1-qt3 dbus-1 )

INSTALL(TARGETS opensyncdbus DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I/usr/local/include/dbus-1.0/qt3 $(all_includes) \
#  -I$(top_srcdir)/kitchensync
#
#bin_PROGRAMS = opensyncdbus
#
#opensyncdbus_SOURCES = dbusclient.cpp
#opensyncdbus_LDADD = $(top_builddir)/kitchensync/libqopensync/libqopensync.la
#opensyncdbus_LDFLAGS = -ldbus-1-qt3 -ldbus-1 $(all_libraries) $(KDE_RPATH)
#
#METASOURCES = AUTO
