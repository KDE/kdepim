kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libkdepim ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kded_networkstatus_PART_SRCS networkstatus.cpp network.cpp )

kde4_automoc(${kded_networkstatus_PART_SRCS})

kde4_add_dcop_skels(kded_networkstatus_PART_SRCS networkstatus.h clientiface.h serviceiface.h )

kde4_add_plugin(kded_networkstatus ${kded_networkstatus_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kded_networkstatus )

target_link_libraries(kded_networkstatus  ${QT_AND_KDECORE_LIBS} kio networkstatus )

install_targets(${LIB_INSTALL_DIR}/kde4 kded_networkstatus )


########### next target ###############


########### next target ###############


########### next target ###############

set(networkstatustestservice_SRCS testservice.cpp )

kde4_automoc(${networkstatustestservice_SRCS})

kde4_add_dcop_skels(networkstatustestservice_SRCS provideriface.h )

kde4_add_dcop_stubs(networkstatustestservice_SRCS serviceiface.h )

kde4_add_executable(networkstatustestservice ${networkstatustestservice_SRCS})

target_link_libraries(networkstatustestservice  ${QT_AND_KDECORE_LIBS} networkstatus )

install_targets(/bin networkstatustestservice )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kded FILES networkstatus.desktop )

kde4_footer()



#original Makefile.am contents follow:

##SUBDIRS = networkstatustray
#
#METASOURCES = AUTO
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#kde_module_LTLIBRARIES = kded_networkstatus.la 
#noinst_LTLIBRARIES = libnetworkstatus.la libconnectionmanager.la
#
#libnetworkstatus_la_LIBADD = $(LIB_KDECORE)
#libnetworkstatus_la_LDFLAGS = $(all_libraries)
#libnetworkstatus_la_SOURCES = networkstatuscommon.cpp
#
#libconnectionmanager_la_LIBADD = $(LIB_KDECORE)
#libconnectionmanager_la_LDFLAGS = $(all_libraries)
#libconnectionmanager_la_SOURCES = connectionmanager.cpp connectionmanager.skel clientiface.stub
#
#INCLUDES = $(all_includes)
#
#kded_networkstatus_la_SOURCES = networkstatus.cpp networkstatus.skel \
#	clientiface.skel serviceiface.skel network.cpp
##kclientiface.skel serviceiface.skel
#kded_networkstatus_la_LIBADD = $(LIB_KDECORE) $(LIB_KIO) ./libnetworkstatus.la
#kded_networkstatus_la_LDFLAGS = $(all_libraries) -module -avoid-version 
#
#servicesdir = $(kde_servicesdir)/kded
#services_DATA = networkstatus.desktop
#
#noinst_HEADERS = serviceiface.h networkstatuscommon.h serviceifaceimpl.h \
#	network.h clientifaceimpl.h provideriface.h testservice.h connectionmanager.h
#
#bin_PROGRAMS = networkstatustestservice
#
#networkstatustestservice_LDFLAGS = $(all_libraries)
#networkstatustestservice_LDADD = libnetworkstatus.la
#
#networkstatustestservice_SOURCES = testservice.cpp provideriface.skel serviceiface.stub
